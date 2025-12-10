#include "TileRenderer.h"

#include "ShaderManager.h"

TileRenderer::TileRenderer() : numTiles(4), tileZ(2) {
    // 创建 tile 网格
    vertices = createTileMesh(32);
    
    // 创建 VAO/VBO
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    glBindVertexArray(0);
    
    // 创建 shader 程序
    shaderProgram = ShaderManager::createProgram();
    
    // 获取 uniform 位置
    u_projection_matrix = glGetUniformLocation(shaderProgram, "u_projection_matrix");
    u_projection_fallback_matrix = glGetUniformLocation(shaderProgram, "u_projection_fallback_matrix");
    u_projection_tile_mercator_coords = glGetUniformLocation(shaderProgram, "u_projection_tile_mercator_coords");
    u_projection_transition = glGetUniformLocation(shaderProgram, "u_projection_transition");
    u_projection_clipping_plane = glGetUniformLocation(shaderProgram, "u_projection_clipping_plane");
    u_color = glGetUniformLocation(shaderProgram, "u_color");
}

TileRenderer::~TileRenderer() {
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);
}

void TileRenderer::render(const GlobeProjection& projection, float aspect)
{
    glUseProgram(shaderProgram);
    glBindVertexArray(VAO);
    
    // 计算裁剪平面（所有 tile 共享）
    Math::Vec4 clippingPlane = projection.calculateClippingPlane();
    Math::Mat4 globeMatrix = projection.calculateGlobeMatrix(aspect);
    
    // 纯 Mercator 模式：渲染所有 wrap
    if (projection.transition < 0.001f)
    {
        for (int wrap = -1; wrap <= 1; wrap++)
        {
            renderTilesForWrap(projection, globeMatrix, clippingPlane, wrap, aspect);
        }
    }
    else
    {
        // Globe 过渡模式：为每个 tile 动态选择最接近 center 的 wrap
        // Globe 部分使用 fract() 归一化，所有 wrap 投影到相同位置（深度测试处理重叠）
        // Mercator 部分需要正确的 wrap 来确保对齐
        for (int tileY = 0; tileY < numTiles; tileY++)
        {
            for (int tileX = 0; tileX < numTiles; tileX++)
            {
                int wrap = projection.getWrapForTile(tileX, tileY, tileZ);
                renderSingleTile(projection, globeMatrix, clippingPlane, tileX, tileY, wrap, aspect);
            }
        }
    }
#if 1
    // 绘制网格线
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    if (projection.transition < 0.001f)
    {
        for (int wrap = -1; wrap <= 1; wrap++)
        {
            renderTilesForWrap(projection, globeMatrix, clippingPlane, wrap, aspect, true);
        }
    }
    else
    {
        for (int tileY = 0; tileY < numTiles; tileY++)
        {
            for (int tileX = 0; tileX < numTiles; tileX++)
            {
                int wrap = projection.getWrapForTile(tileX, tileY, tileZ);
                renderSingleTile(projection, globeMatrix, clippingPlane, tileX, tileY, wrap, aspect, true);
            }
        }
    }
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
}

void TileRenderer::renderTilesForWrap(const GlobeProjection& projection, const Math::Mat4& globeMatrix, const Math::Vec4& clippingPlane, int wrap, float aspect, bool wireframe)
{
    for (int tileY = 0; tileY < numTiles; tileY++)
    {
        for (int tileX = 0; tileX < numTiles; tileX++)
        {
            renderSingleTile(projection, globeMatrix, clippingPlane, tileX, tileY, wrap, aspect, wireframe);
        }
    }
}

void TileRenderer::renderSingleTile(const GlobeProjection& projection, const Math::Mat4& globeMatrix,const Math::Vec4& clippingPlane, int tileX, int tileY, int wrap, float aspect, bool wireframe)
{
    Math::Mat4 mercatorMatrix = projection.calculateMercatorMatrix(tileX, tileY, tileZ, wrap, aspect);
    Math::Vec4 tileMercCoords = projection.calculateTileMercatorCoords(tileX, tileY, tileZ, wrap);
    
    // 设置 uniforms
    glUniformMatrix4fv(u_projection_matrix, 1, GL_FALSE, globeMatrix.data());
    glUniformMatrix4fv(u_projection_fallback_matrix, 1, GL_FALSE, mercatorMatrix.data());
    glUniform4fv(u_projection_tile_mercator_coords, 1, tileMercCoords.data());
    glUniform1f(u_projection_transition, projection.transition);
    glUniform4fv(u_projection_clipping_plane, 1, clippingPlane.data());
    
    // 颜色
    if (wireframe)
    {
        glUniform4f(u_color, 0.0f, 0.0f, 0.0f, 1.0f);
    }
    else
    {
        float r = ((tileX + tileY) % 2 == 0) ? 0.3f : 0.5f;
        float g = ((tileX + tileY) % 2 == 0) ? 0.5f : 0.3f;
        glUniform4f(u_color, r, g, 0.4f, 1.0f);
    }
    
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 2);
}

std::vector<float> TileRenderer::createTileMesh(int divisions)
{
    std::vector<float> verts;
    float step = Constants::TILE_EXTENT / (float)divisions;
    
    for (int y = 0; y < divisions; y++)
    {
        for (int x = 0; x < divisions; x++)
        {
            float x0 = x * step;
            float y0 = y * step;
            float x1 = (x + 1) * step;
            float y1 = (y + 1) * step;
            
            // 两个三角形
            verts.insert(verts.end(), {x0, y0, x1, y0, x0, y1});
            verts.insert(verts.end(), {x1, y0, x1, y1, x0, y1});
        }
    }
    return verts;
}


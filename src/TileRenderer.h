#pragma once
#include "GlobeProjection.h"
#include "glad/glad.h"
#include <glm/glm.hpp>
#include <vector>

class TileRenderer {
private:
    GLuint shaderProgram;
    GLuint VAO, VBO;
    GLuint u_projection_matrix;
    GLuint u_projection_fallback_matrix;
    GLuint u_projection_tile_mercator_coords;
    GLuint u_projection_transition;
    GLuint u_projection_clipping_plane;
    GLuint u_color;
    
    std::vector<float> vertices;
    int numTiles;
    int tileZ;
    
public:
    TileRenderer();
    ~TileRenderer();
    
    /**
     * 渲染所有 tile
     * 
     * 关键逻辑：
     * 1. 纯 Mercator 模式：渲染 wrap=-1, 0, 1（传统 Mercator 行为）
     * 2. Globe 过渡模式：为每个 tile 动态选择 wrap（避免重复和缺失）
     */
    void render(const GlobeProjection& projection, float aspect);
    
private:
    void renderTilesForWrap(const GlobeProjection& projection, const glm::mat4& globeMatrix, const glm::vec4& clippingPlane, int wrap, float aspect, bool wireframe = false);
    void renderSingleTile(const GlobeProjection& projection, const glm::mat4& globeMatrix,const glm::vec4& clippingPlane, int tileX, int tileY, int wrap, float aspect, bool wireframe = false);
    
    static std::vector<float> createTileMesh(int divisions = 32);
};

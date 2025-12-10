#include "GlobeProjection.h"

#include <algorithm>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

float GlobeProjection::getCameraDistance() const
{
    return 2.0f + 4.0f / pow(2.0f, zoom * 0.5f);
}

float GlobeProjection::getGlobeRadius() const
{
    float worldScale = 2.0f * pow(2.0f, zoom);
    return worldScale / (2.0f * Constants::PI);  // 周长 = worldScale
}

float GlobeProjection::wrapLon(float lon) const
{
    lon = fmod(lon, 360.0f);
    if (lon > 180.0f) lon -= 360.0f;
    if (lon < -180.0f) lon += 360.0f;
    return lon;
}

glm::mat4 GlobeProjection::calculateGlobeMatrix(float aspect) const
{
    float dist = getCameraDistance();
    float globeRadius = getGlobeRadius();
    
    glm::mat4 proj = glm::perspective(Constants::PI / 4.0f, aspect, 0.01f, 100.0f);
    
    float wrappedLon = wrapLon(centerLon);
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -dist))
                     * glm::rotate(glm::mat4(1.0f), centerLat * Constants::PI / 180.0f, glm::vec3(1.0f, 0.0f, 0.0f))
                     * glm::rotate(glm::mat4(1.0f), -wrappedLon * Constants::PI / 180.0f, glm::vec3(0.0f, 1.0f, 0.0f))
                     * glm::scale(glm::mat4(1.0f), glm::vec3(globeRadius, globeRadius, globeRadius));
    
    return proj * view;
}

glm::mat4 GlobeProjection::calculateMercatorMatrix(int tileX, int tileY, int tileZ, int wrap, float aspect) const
{
    float dist = getCameraDistance();
    
    float numTiles = pow(2.0f, tileZ);
    float tileOffsetX = (tileX + wrap * numTiles) / numTiles;
    float tileOffsetY = tileY / numTiles;
    float tileScaleInv = 1.0f / numTiles;
    
    // 使用 wrapped centerLon 确保在 -180..180 范围内
    float wrappedLon = wrapLon(centerLon);
    float centerMercX = wrappedLon / 360.0f + 0.5f;
    float latRad = centerLat * Constants::PI / 180.0f;
    float centerMercY = 0.5f - log(tan(Constants::PI/4.0f + latRad/2.0f)) / (2.0f * Constants::PI);
    
    // 缩放因子使平面和球体在屏幕上大小匹配
    float worldScale = 2.0f * pow(2.0f, zoom);
    
    // Model 矩阵：缩放 -> 居中 -> tile 偏移 -> tile 缩放
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(worldScale, -worldScale, 1.0f))  // Y 轴翻转，缩放
                       * glm::translate(glm::mat4(1.0f), glm::vec3(-centerMercX, -centerMercY, 0.0f))  // 居中到 center
                       * glm::translate(glm::mat4(1.0f), glm::vec3(tileOffsetX, tileOffsetY, 0.0f))    // tile 偏移
                       * glm::scale(glm::mat4(1.0f), glm::vec3(tileScaleInv / Constants::TILE_EXTENT, tileScaleInv / Constants::TILE_EXTENT, 1.0f));
    
    glm::mat4 view = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, -dist));
    glm::mat4 proj = glm::perspective(Constants::PI / 4.0f, aspect, 0.01f, 100.0f);
    return proj * view * model;
}

glm::vec4 GlobeProjection::calculateTileMercatorCoords(int tileX, int tileY, int tileZ, int wrap) const
{
    float numTiles = pow(2.0f, tileZ);
    return glm::vec4(
        (tileX + wrap * numTiles) / numTiles,
        tileY / numTiles,
        1.0f / numTiles / Constants::TILE_EXTENT,
        1.0f / numTiles / Constants::TILE_EXTENT
        );
}

int GlobeProjection::getWrapForTile(int tileX, int tileY, int tileZ) const
{
    // center 在归一化 Mercator 空间的位置
    // 关键：使用 wrapped centerLon 确保 centerMercX 在 [0, 1) 范围内
    float wrappedLon = wrapLon(centerLon);
    float centerMercX = wrappedLon / 360.0f + 0.5f;
    
    float numTiles = pow(2.0f, tileZ);
    float tileMercSize = 1.0f / numTiles;
    float tileX_merc = tileX / numTiles; // tile 在归一化 Mercator 空间的位置 [0, 1)
    
    // 计算 center 到 tile 在三个不同 wrap 下的距离
    auto distanceToTile = [](float point, float tile, float tileSize) -> float {
        float delta = point - tile;
        return (delta < 0) ? -delta : std::max(0.0f, delta - tileSize);
    };
    
    // 三个 wrap 选项：
    // wrap=0: tileX_merc (当前)
    // wrap=-1: tileX_merc - 1.0 (左侧副本)
    // wrap=1: tileX_merc + 1.0 (右侧副本)
    float distCurrent = distanceToTile(centerMercX, tileX_merc, tileMercSize);
    float distLeft = distanceToTile(centerMercX, tileX_merc - 1.0f, tileMercSize);
    float distRight = distanceToTile(centerMercX, tileX_merc + 1.0f, tileMercSize);
    
    // 选择距离最小的 wrap（maplibre 标准算法）
    // maplibre 不使用 fract()，不同 wrap 的 tile 在 Globe 中投影到不同的球面位置
    // 通过为每个 tile 选择最接近 center 的 wrap，确保每个球面位置只有一个 tile 渲染
    float distSmallest = std::min({distCurrent, distLeft, distRight});
    if (distSmallest == distRight)
    {
        return 1;
    }
    if (distSmallest == distLeft)
    {
        return -1;
    }
    return 0;
}

glm::vec4 GlobeProjection::calculateClippingPlane() const
{
    float dist = getCameraDistance();
    float globeRadius = getGlobeRadius();
    
    // 关键：将摄像机距离转换为相对于单位球的距离
    float distanceCameraToB = dist / globeRadius;
    float radius = 1.0f;  // 单位球
    
    // pitch = 0 的简化情况
    float pitch = 0.0f;
    
    // Distance from camera to "A"
    float distanceCameraToA = sin(pitch) * distanceCameraToB;  // = 0 when pitch=0
    // Distance from "A" to "C"
    float distanceAtoC = cos(pitch) * distanceCameraToB + radius;  // = dist/r + 1
    // Distance from camera to "C"
    float distanceCameraToC = sqrt(distanceCameraToA * distanceCameraToA + distanceAtoC * distanceAtoC);
    // cam - C - T angle cosine
    float camCTcosine = radius / distanceCameraToC;
    // Distance from globe center to tangent plane
    float tangentPlaneDistanceToC = camCTcosine * radius;
    
    // Vector from C to cam (normalized)
    float vectorCtoCamX = -distanceCameraToA;
    float vectorCtoCamY = distanceAtoC;
    float vectorCtoCamLength = sqrt(vectorCtoCamX * vectorCtoCamX + vectorCtoCamY * vectorCtoCamY);
    vectorCtoCamX /= vectorCtoCamLength;
    vectorCtoCamY /= vectorCtoCamLength;
    
    // planeVector = [0, vectorCtoCamX, vectorCtoCamY]
    // 对于 pitch=0: [0, 0, 1]
    float px = 0.0f;
    float py = vectorCtoCamX;  // = 0 for pitch=0
    float pz = vectorCtoCamY;   // = 1 for pitch=0
    
    // 应用旋转：rotateX(-lat) 然后 rotateY(lon) - 使用 wrap 后的经度
    float latRad = centerLat * Constants::PI / 180.0f;
    float lonRad = wrapLon(centerLon) * Constants::PI / 180.0f;
    
    // rotateX(-lat): 绕 X 轴旋转
    float cosLat = cos(-latRad);
    float sinLat = sin(-latRad);
    float py1 = py * cosLat - pz * sinLat;
    float pz1 = py * sinLat + pz * cosLat;
    py = py1;
    pz = pz1;
    
    // rotateY(lon): 绕 Y 轴旋转
    float cosLon = cos(lonRad);
    float sinLon = sin(lonRad);
    float px1 = px * cosLon + pz * sinLon;
    float pz2 = -px * sinLon + pz * cosLon;
    px = px1;
    pz = pz2;
    
    // 归一化
    float len = sqrt(px*px + py*py + pz*pz);
    px /= len;
    py /= len;
    pz /= len;
    
    return glm::vec4(px, py, pz, -tangentPlaneDistanceToC / len);
}


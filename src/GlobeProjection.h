#pragma once
#include "Constants.h"
#include <glm/glm.hpp>

class GlobeProjection
{
public:
    float transition = 0.0f;      // 过渡因子 (0=墨卡托, 1=Globe)
    float centerLon = 0.0f;       // 中心经度（允许连续旋转，不 wrap）
    float centerLat = 0.0f;       // 中心纬度
    float zoom = 2.0f;            // 缩放级别
    
    /**
     * 获取共享的摄像机距离
     * 关键：Globe 和 Mercator 使用相同的相机距离，确保屏幕中心对齐
     */
    float getCameraDistance() const;
    
    /**
     * 获取 Globe 半径（与 Mercator 的 worldScale 匹配）
     * 确保 Globe 和 Mercator 在屏幕上的大小一致
     */
    float getGlobeRadius() const;
    
    /**
     * 将经度 wrap 到 -180..180 范围（仅用于计算，不影响连续旋转）
     */
    float wrapLon(float lon) const;
    
    /**
     * 计算 Globe 投影矩阵
     * 
     * 关键点：
     * 1. 使用 wrap 后的经度确保矩阵计算正确
     * 2. 应用缩放使 Globe 半径与 Mercator worldScale 匹配
     * 3. 旋转顺序：translate -> rotateX(lat) -> rotateY(-lon) -> scale
     */
    glm::mat4 calculateGlobeMatrix(float aspect) const;
    
    /**
     * 计算 Mercator 投影矩阵
     * 
     * 关键点：
     * 1. 使用 wrap 后的 centerLon 确保在 -180..180 范围内
     * 2. worldScale 与 Globe 半径匹配，确保屏幕大小一致
     * 3. 支持 wrap 参数，用于渲染不同世界副本的 tile
     */
    glm::mat4 calculateMercatorMatrix(int tileX, int tileY, int tileZ, int wrap, float aspect) const;
    
    /**
     * 计算 Tile 的墨卡托坐标（归一化 0..1）
     * 
     * 返回值：[offsetX, offsetY, scaleX, scaleY]
     * 用于 shader 中将 tile 坐标转换为归一化墨卡托坐标
     */
    glm::vec4 calculateTileMercatorCoords(int tileX, int tileY, int tileZ, int wrap) const;
    
    /**
     * 动态 Wrap 选择（maplibre 核心算法）
     * 
     * 为每个 tile 选择最接近 center 的 wrap 值 (-1, 0, 1)
     * 
     * 关键点：
     * 1. 计算 center 到 tile 在三个不同 wrap 下的距离
     * 2. 选择距离最小的 wrap
     * 3. 这样既能完整覆盖地图，又避免重复球体
     * 
     * maplibre 的实现：
     * - shader 中不使用 fract()，不同 wrap 的 tile 在 Globe 中投影到不同的球面位置
     * - 通过为每个 tile 选择最接近 center 的 wrap，确保每个球面位置只有一个 tile 渲染
     * - 这样既能完整覆盖，又避免重复
     */
    int getWrapForTile(int tileX, int tileY, int tileZ) const;
    
    /**
     * 计算裁剪平面（maplibre 标准实现）
     * 
     * 用于 Globe 模式下隐藏背面几何体
     * 返回平面方程：[nx, ny, nz, d]，其中 dot(pos, [nx,ny,nz]) + d = 0
     */
    glm::vec4 calculateClippingPlane() const;
};

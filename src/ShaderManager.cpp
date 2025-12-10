#include "ShaderManager.h"

#include <iostream>

namespace
{
const char* kVertexShaderSource = R"(
#version 330 core

layout(location = 0) in vec2 a_pos;

// 双矩阵系统：Globe 和 Mercator 投影矩阵
uniform mat4 u_projection_matrix;              // Globe 投影矩阵（投影单位球）
uniform mat4 u_projection_fallback_matrix;      // Mercator 投影矩阵（投影 tile 坐标）
uniform vec4 u_projection_tile_mercator_coords; // Tile 墨卡托坐标: [offsetX, offsetY, scaleX, scaleY]
uniform float u_projection_transition;          // 过渡因子 (0=墨卡托, 1=Globe)
uniform vec4 u_projection_clipping_plane;      // 裁剪平面（用于 Globe 背面裁剪）

#define PI 3.14159265358979323846

/**
 * 将 tile 坐标转换为单位球面坐标
 * maplibre 通过 tile 选择层面的 wrap 控制来避免重复，而不是在 shader 中归一化
 */
vec3 projectToSphere(vec2 posInTile) {
    // tile 坐标 -> 归一化墨卡托
    vec2 mercatorPos = u_projection_tile_mercator_coords.xy + 
                       u_projection_tile_mercator_coords.zw * posInTile;
    
    // 归一化墨卡托 -> 球面角度
    vec2 spherical;
    spherical.x = mercatorPos.x * PI * 2.0 + PI;  // 经度
    spherical.y = 2.0 * atan(exp(PI - mercatorPos.y * PI * 2.0)) - PI * 0.5;  // 纬度
    
    // 球面角度 -> 单位球笛卡尔坐标
    float len = cos(spherical.y);
    return vec3(
        sin(spherical.x) * len,
        sin(spherical.y),
        cos(spherical.x) * len
    );
}

/**
 * 计算用于裁剪背面的 Z 值（maplibre 标准实现）
 * 使用裁剪平面方程判断点是否在球体可见侧
 */
float globeComputeClippingZ(vec3 spherePos) {
    return (1.0 - (dot(spherePos, u_projection_clipping_plane.xyz) + u_projection_clipping_plane.w));
}

void main() {
    // 计算球面坐标
    vec3 spherePos = projectToSphere(a_pos);
    
    // Globe 裁剪空间坐标
    vec4 globePosition = u_projection_matrix * vec4(spherePos, 1.0);
    // 关键：用自定义 Z 替换，用于裁剪背面
    globePosition.z = globeComputeClippingZ(spherePos) * globePosition.w;
    
    // 完全 Globe 模式：直接使用 Globe 投影
    if (u_projection_transition > 0.999) {
        gl_Position = globePosition;
        return;
    }
    
    // Mercator 裁剪空间坐标
    vec4 flatPosition = u_projection_fallback_matrix * vec4(a_pos, 0.0, 1.0);
    
    // 关键：Z 值延迟混合策略（maplibre 标准实现）
    // 前 80% 过渡：Z 保持为 0（Mercator 的平面深度）
    // 后 20% 过渡：逐渐使用 Globe 的自定义 Z（用于裁剪背面）
    // 原因：Globe 的背面裁剪 Z 在过渡早期会导致不正确的深度
    const float z_globeness_threshold = 0.2;
    float zMix = clamp((u_projection_transition - z_globeness_threshold) / (1.0 - z_globeness_threshold), 0.0, 1.0);
    
    vec4 result = globePosition;
    
    // maplibre 标准实现：直接混合 Z 值
    // globePosition.z 已经是 globeComputeClippingZ * w
    // 对于背面的点，globeComputeClippingZ 返回很小的值或负数
    // 在深度测试中，这些点会被自然过滤掉（如果深度测试设置为 GL_LESS）
    result.z = mix(0.0, globePosition.z, zMix);
    
    // XYW 组件：在裁剪空间直接混合（避免在世界空间混合导致的扭曲）
    result.xyw = mix(flatPosition.xyw, globePosition.xyw, u_projection_transition);
    
    gl_Position = result;
}
)";

const char* kFragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
uniform vec4 u_color;
void main() {
    FragColor = u_color;
}
)";
} // namespace

const char* ShaderManager::getVertexShaderSource()
{
    return kVertexShaderSource;
}

const char* ShaderManager::getFragmentShaderSource()
{
    return kFragmentShaderSource;
}

GLuint ShaderManager::compileShader(GLenum type, const char* source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);
    
    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, 512, nullptr, log);
        std::cerr << "Shader compilation error: " << log << std::endl;
    }
    return shader;
}

GLuint ShaderManager::createProgram()
{
    GLuint program = glCreateProgram();
    glAttachShader(program, compileShader(GL_VERTEX_SHADER, getVertexShaderSource()));
    glAttachShader(program, compileShader(GL_FRAGMENT_SHADER, getFragmentShaderSource()));
    glLinkProgram(program);
    return program;
}


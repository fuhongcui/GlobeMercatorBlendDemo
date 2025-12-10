#pragma once
#include <cmath>
#include <cstring>

namespace Constants
{
constexpr float PI = 3.14159265358979323846f;
constexpr int TILE_EXTENT = 8192;
constexpr float Z_GLOBENESS_THRESHOLD = 0.2f;  // Z 值混合阈值（maplibre 标准）
}

namespace Math
{

struct Vec4
{
    float x, y, z, w;
    Vec4(float a = 0, float b = 0, float c = 0, float d = 1) : x(a), y(b), z(c), w(d)
    {

    }
    const float* data() const
    {
        return &x;
    }
};

struct Mat4
{
    float m[16];

    Mat4()
    {
        identity();
    }

    void identity()
    {
        std::memset(m, 0, sizeof(m));
        m[0] = m[5] = m[10] = m[15] = 1.0f;
    }

    float& operator()(int row, int col)
    {
        return m[col * 4 + row];
    }
    float operator()(int row, int col) const
    {
        return m[col * 4 + row];
    }

    Mat4 operator*(const Mat4& other) const
    {
        Mat4 result;
        for (int i = 0; i < 4; i++)
        {
            for (int j = 0; j < 4; j++)
            {
                result(i, j) = 0;
                for (int k = 0; k < 4; k++)
                {
                    result(i, j) += (*this)(i, k) * other(k, j);
                }
            }
        }
        return result;
    }

    const float* data() const
    {
        return m;
    }
};

// 透视投影矩阵
Mat4 perspective(float fov, float aspect, float near, float far)
{
    Mat4 m;
    float tanHalfFov = tan(fov / 2.0f);
    m(0, 0) = 1.0f / (aspect * tanHalfFov);
    m(1, 1) = 1.0f / tanHalfFov;
    m(2, 2) = -(far + near) / (far - near);
    m(2, 3) = -2.0f * far * near / (far - near);
    m(3, 2) = -1.0f;
    m(3, 3) = 0.0f;
    return m;
}

// 平移矩阵
Mat4 translate(float x, float y, float z)
{
    Mat4 m;
    m(0, 3) = x;
    m(1, 3) = y;
    m(2, 3) = z;
    return m;
}

// 旋转矩阵
Mat4 rotateX(float angle)
{
    Mat4 m;
    float c = cos(angle), s = sin(angle);
    m(1, 1) = c; m(1, 2) = -s;
    m(2, 1) = s; m(2, 2) = c;
    return m;
}

Mat4 rotateY(float angle)
{
    Mat4 m;
    float c = cos(angle), s = sin(angle);
    m(0, 0) = c; m(0, 2) = s;
    m(2, 0) = -s; m(2, 2) = c;
    return m;
}

// 缩放矩阵
Mat4 scale(float x, float y, float z)
{
    Mat4 m;
    m(0, 0) = x;
    m(1, 1) = y;
    m(2, 2) = z;
    return m;
}


}


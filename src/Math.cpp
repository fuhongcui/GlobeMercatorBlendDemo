#include "Math.h"

namespace Math
{

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

Mat4 translate(float x, float y, float z)
{
    Mat4 m;
    m(0, 3) = x;
    m(1, 3) = y;
    m(2, 3) = z;
    return m;
}

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

Mat4 scale(float x, float y, float z)
{
    Mat4 m;
    m(0, 0) = x;
    m(1, 1) = y;
    m(2, 2) = z;
    return m;
}

} // namespace Math


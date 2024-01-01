#pragma once

#include <cmath>

#include "vec3f.hpp"

class Mat4x4f
{
private:
    float m_values[4][4];
    Mat4x4f(){
        // Private constructor to prevent default initialization
    };

public:
    Mat4x4f(
        float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33)
    {
        m_values[0][0] = m00;
        m_values[0][1] = m01;
        m_values[0][2] = m02;
        m_values[0][3] = m03;

        m_values[1][0] = m10;
        m_values[1][1] = m11;
        m_values[1][2] = m12;
        m_values[1][3] = m13;

        m_values[2][0] = m20;
        m_values[2][1] = m21;
        m_values[2][2] = m22;
        m_values[2][3] = m23;

        m_values[3][0] = m30;
        m_values[3][1] = m31;
        m_values[3][2] = m32;
        m_values[3][3] = m33;
    }

    static Mat4x4f identity()
    {
        return Mat4x4f(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1);
    }

    static Mat4x4f uniform_scale(float s)
    {
        return Mat4x4f(
            s, 0, 0, 0,
            0, s, 0, 0,
            0, 0, s, 0,
            0, 0, 0, 1);
    }

    static Mat4x4f scale(const Vec3f &v)
    {
        return Mat4x4f(
            v.x, 0, 0, 0,
            0, v.y, 0, 0,
            0, 0, v.z, 0,
            0, 0, 0, 1);
    }

    static Mat4x4f rotation_axis_angle(const Vec3f &axis, float angle)
    {
        float c = std::cos(angle);
        float s = std::sin(angle);
        float t = 1 - c;

        float x = axis.x;
        float y = axis.y;
        float z = axis.z;

        return Mat4x4f(
            t * x * x + c, t * x * y + z * s, t * x * z - y * s, 0,
            t * x * y - z * s, t * y * y + c, t * y * z + x * s, 0,
            t * x * z + y * s, t * y * z - x * s, t * z * z + c, 0,
            0, 0, 0, 1);
    }

    static Mat4x4f translation(const Vec3f &v)
    {
        return Mat4x4f(
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            v.x, v.y, v.z, 1);
    }

    static Mat4x4f ortho(float left, float right, float bottom, float top, float near, float far)
    {
        return Mat4x4f(
            2 / (right - left), 0, 0, 0,
            0, 2 / (top - bottom), 0, 0,
            0, 0, -2 / (far - near), 0,
            -(right + left) / (right - left), -(top + bottom) / (top - bottom), -(far + near) / (far - near), 1);
    }

    static Mat4x4f perspective(float fov, float aspect_ratio, float near, float far)
    {
        float tan_half_fov = std::tan(fov / 2);
        return Mat4x4f(
            1 / (aspect_ratio * tan_half_fov), 0, 0, 0,
            0, 1 / tan_half_fov, 0, 0,
            0, 0, -(far + near) / (far - near), -1,
            0, 0, -(2 * far * near) / (far - near), 0);
    }

    Mat4x4f operator*(const Mat4x4f &other) const
    {
        Mat4x4f result;
        for (int col = 0; col < 4; col++)
        {
            for (int row = 0; row < 4; row++)
            {
                result.m_values[col][row] = 0;
                for (int i = 0; i < 4; i++)
                {
                    result.m_values[col][row] += m_values[i][row] * other.m_values[col][i];
                }
            }
        }
        return result;
    }

    float *values_ptr()
    {
        return (float *)m_values;
    }
};

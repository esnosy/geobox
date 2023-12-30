#pragma once

#include <cmath>

struct Vec3f
{
    float x;
    float y;
    float z;

    Vec3f cross(const Vec3f &other) const
    {
        return Vec3f{
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x,
        };
    }

    Vec3f operator/(float s) const { return Vec3f{x / s, y / s, z / s}; }
    friend Vec3f operator/(float s, const Vec3f &v) { return Vec3f{s / v.x, s / v.y, s / v.z}; }
    Vec3f operator*(float s) const { return Vec3f{x * s, y * s, z * s}; }
    friend Vec3f operator*(float s, const Vec3f &v) { return v * s; }
    float calc_magnitude() const { return std::sqrt(x * x + y * y + z * z); }
    void normalize()
    {
        float m = calc_magnitude();
        x /= m;
        y /= m;
        z /= m;
    }
    Vec3f operator-(const Vec3f &other) const { return Vec3f{x - other.x, y - other.y, z - other.z}; }
    Vec3f operator+(const Vec3f &other) const { return Vec3f{x + other.x, y + other.y, z + other.z}; }
};
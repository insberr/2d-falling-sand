//
// Created by jonah on 5/15/2024.
//


#pragma once

struct vec3 {
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3(float x, float y) : x(x), y(y), z(0.0f) {}
    explicit vec3(float x) : x(x), y(x), z(x) {}

    vec3 floor() const;
    vec3 ceil() const;
    vec3 normalize() const;

    vec3 operator+(const vec3& rhs) const;

    vec3 operator *(float scale) const;
    vec3 operator *(const vec3& vec) const;
    vec3& operator*=(float scale);

    float x;
    float y;
    float z;
};

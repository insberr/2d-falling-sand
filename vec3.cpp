//
// Created by jonah on 5/15/2024.
//

#include "vec3.h"

#include <cmath>

vec3 vec3::floor() const {
    return {
        std::floorf(x),
        std::floorf(y),
        std::floorf(z)
    };
}

vec3 vec3::ceil() const {
    return {
        std::ceilf(x),
        std::ceilf(y),
        std::ceilf(z)
    };
}

vec3 vec3::operator+(const vec3 &rhs) const {
    return {
        this->x + rhs.x,
        this->y + rhs.y,
        this->z + rhs.z
    };
}

vec3 vec3::operator*(float scale) const {
    return vec3{
        this->x * scale,
        this->y * scale,
        this->z * scale
    };
}

vec3 vec3::normalize() const {
    float magnitude = std::sqrtf(
        (x * x) + (y * y) + (z * z)
    );
    return {
        x * (1 / magnitude),
        y * (1 / magnitude),
        z * (1 / magnitude)
    };
}


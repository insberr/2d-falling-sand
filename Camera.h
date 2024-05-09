//
// Created by jonah on 4/14/2024.
//


#pragma once
#include "Graphics.h"

constexpr float PI = 3.14159265f;
constexpr double PI_D = 3.1415926535897932;

template<typename T>
T wrap_angle( T theta )
{
    const T modded = fmod( theta,(T)2.0 * (T)PI_D );
    return (modded > (T)PI_D) ?
           (modded - (T)2.0 * (T)PI_D) :
           modded;
}

class Camera {
public:
    Camera() noexcept;

    DirectX::XMMATRIX GetMatrix() const noexcept;
    void SpawnControlWindow() noexcept;
    void Reset() noexcept;
    void Rotate(float dx, float dy) noexcept;
    void Translate(DirectX::XMFLOAT3 translation) noexcept;
private:
    DirectX::XMFLOAT3 pos;
    float pitch;
    float yaw;
    static constexpr float travelSpeed = 20.0f;
    static constexpr float rotationSpeed = 0.002f;
};

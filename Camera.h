//
// Created by jonah on 4/14/2024.
//


#pragma once
#include "Graphics.h"

class Camera {
public:
    DirectX::XMMATRIX GetMatrix() const noexcept;
    void SpawnControlWindow() noexcept;
    void Reset() noexcept;
private:
    float r = 80.0f;
    float theta = 0.0f;
    float phi = 0.0f;
    float pitch = 0.0f;
    float yaw = 0.0f;
    float roll = 3.14159f;
};

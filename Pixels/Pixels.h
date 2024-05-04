//
// Created by jonah on 5/2/2024.
//

#pragma once

#include "../Graphics.h"
#include "../Window.h"
#include "../EngineTimer.h"
#include <map>

namespace wrl = Microsoft::WRL;

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct Position {
    int x;
    int y;
    bool operator< (const Position& position) const {
        if (this->y > position.y) {
            return true;
        }
        if (this->y == position.y && this->x < position.x) {
            return true;
        }
        return false;
    }
};

class Pixel {
    friend class Pixels;
public:
    enum class Type {
        Unknown = 0,
        Sand,
        Water,
        Lava,
        Rock,

        last
    };

    Pixel(Type type_): type(type_) {}
    ~Pixel() = default;

    Type GetType() const {
        return type;
    }
private:
    Type type{Type::Unknown};
};

Color ColorForPixel(const Pixel& pixel);

class Pixels {
public:
    Pixels(Graphics &gfx);

    Pixels();

    void Update(Window &wnd, float dt);

    void UpdateConstantBuffer(Graphics &gfx);
    void DrawUI(Graphics &gfx);
    void Draw(Graphics &gfx);

    ~Pixels();
private:
    const float WindowWidth = 800.0f;
    const float WindowHeight = 600.0f;
    // Simulation controls
    float PixelSize = 1.0f;
    bool BottomStop = true;
    Pixel::Type particleDrawType{Pixel::Type::Sand};
    // Constants calculated based on simulation controls
    unsigned int GridWidth = static_cast<unsigned int>(WindowWidth / PixelSize);
    unsigned int GridHeight = static_cast<unsigned int>(WindowHeight / PixelSize);


    // Graphics Buffers
    wrl::ComPtr<ID3D11Buffer> vertexBuffer;
    wrl::ComPtr<ID3D11Buffer> instanceBuffer;

    // The pixels and simulation
    std::map<Position, Pixel*> pixels;
    float stepSpeed{0.01f};
    float stepTime{stepSpeed};

    // Speed Monitoring
    EngineTimer updateTime;
    EngineTimer renderTime;
};

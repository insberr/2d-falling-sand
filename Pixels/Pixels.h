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

struct PixelInstance {
    struct {
        float x;
        float y;
    } worldPosition;
    struct {
        float r;
        float g;
        float b;
        float a;
    } color;
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

    PixelInstance GetInstance(const Position& pos, unsigned int GridWidth, unsigned int GridHeight);

    Color GetColor();
private:
    Type type{Type::Unknown};
};

struct Timing {
    std::string name;
    float time;
};

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
    const float WindowWidth = 1280.0f;
    const float WindowHeight = 720.0f;
    // Simulation controls
    float PixelSize = 2.0f;
    bool BottomStop = true;
    // -- Drawing
    Pixel::Type particleDrawType{Pixel::Type::Sand};
    unsigned int drawSize{1};
    // Constants calculated based on simulation controls
    unsigned int GridWidth = static_cast<unsigned int>(WindowWidth / PixelSize);
    unsigned int GridHeight = static_cast<unsigned int>(WindowHeight / PixelSize);


    // Graphics Buffers
    wrl::ComPtr<ID3D11Buffer> vertexBuffer;
    wrl::ComPtr<ID3D11Buffer> instanceBuffer;

    // The pixels and simulation
    std::map<Position, std::shared_ptr<Pixel>> pixels;
    std::vector<PixelInstance> pixelInstances;
    float stepSpeed{0.01f};
    float stepTime{stepSpeed};

    // Speed Monitoring
    EngineTimer updateTime;
    float timeTakenUpdate{0.0f};
    std::vector<Timing>updateTimings;
    EngineTimer renderTime;
    float timeTakenRender{0.0f};
};

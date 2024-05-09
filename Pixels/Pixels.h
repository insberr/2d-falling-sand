//
// Created by jonah on 5/2/2024.
//

#pragma once

#include "../Graphics.h"
#include "../Window.h"
#include "../Camera.h"
#include "../EngineTimer.h"
#include <map>

namespace wrl = Microsoft::WRL;

struct NormalizedColor {
    float r;
    float g;
    float b;
    float a;
};

struct Color {
    NormalizedColor normalize() const;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct Position {
    Position(int x, int y, int z) : y(y), x(x), z(z) {}
    Position(float x, float y, float z) : y(y), x(x), z(z) {}

    // Y MUST be first since I want the map to be sorted by y first
    // This is important for how the particles visually update
    float y;
    float x;
    float z;
    auto operator<=> (const Position& position) const = default;
    // {
    //     if (this->y > position.y && this->x == position.x && this->z == position.z) {
    //         return true;
    //     }
    //     if (this->y == position.y && this->x < position.x && this->z < position.z) {
    //         return true;
    //     }
    //     if (this->y == position.y && this->x < position.x && this->z == position.z) {
    //         return true;
    //     }
    //     if (this->y == position.y && this->x == position.x && this->z < position.z) {
    //         return true;
    //     }
    //     return false;
    // }
};

struct PixelInstance {
    struct {
        float x;
        float y;
        float z;
    } worldPosition;
    NormalizedColor color;
};

struct vec3 {
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    vec3(float x, float y) : x(x), y(y), z(0.0f) {}
    explicit vec3(float x) : x(x), y(x), z(x) {}

    vec3 floor() const;
    vec3 ceil() const;

    vec3 operator+(const vec3& rhs) const;

    float x;
    float y;
    float z;
};

class Pixel {
    friend class Pixels;
public:
    enum class Type {
        Unknown = 0,
        Debug,
        
        Sand,
        Water,
        Lava,
        Rock,

        last
    };

    Pixel(Type type_, Position pos): type(type_) {
        realPosition = {
            pos.x,
            pos.y,
            pos.z
        };
    }
    ~Pixel() = default;

    Type GetType() const {
        return type;
    }

    PixelInstance GetInstance(const Position& pos, unsigned int GridWidth, unsigned int GridHeight, unsigned int GridDepth);

    Color GetColor() const;
    vec3 Velocity() const;

    vec3 realPosition { 0 };
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

    void Update(Window &wnd, Camera &cam, float dt);

    void UpdateConstantBuffer(Graphics &gfx);

    void Update_Drawing(Window &wnd);

    void DrawUI(Graphics &gfx);
    void Draw(Graphics &gfx);

    ~Pixels();
private:
    // const float WindowWidth = 1280.0f;
    // const float WindowHeight = 720.0f;
    // const float WindowDepth = 720.0f;
    // Simulation controls
    float PixelSize = 1.0f;
    bool BottomStop = true;
    // -- Drawing
    Pixel::Type particleDrawType { Pixel::Type::Sand };
    unsigned int drawSize { 1 };
    bool drawingEnabled { false };
    // Constants calculated based on simulation controls
    unsigned int GridWidth  { 9 * 5 };
    unsigned int GridHeight { 9 * 5 };
    unsigned int GridDepth  { 9 * 5 };


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

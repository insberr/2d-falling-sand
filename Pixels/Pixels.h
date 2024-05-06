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
    Position(int x, int y, int z) : y(y), x(x), z(z) {}

    int y;
    int x;
    int z;
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
    struct {
        float r;
        float g;
        float b;
        float a;
    } color;
    // float padding;
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

    PixelInstance GetInstance(const Position& pos, unsigned int GridWidth, unsigned int GridHeight, unsigned int GridDepth);

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
    // const float WindowWidth = 1280.0f;
    // const float WindowHeight = 720.0f;
    // const float WindowDepth = 720.0f;
    // Simulation controls
    float PixelSize = 1.0f;
    bool BottomStop = true;
    // -- Drawing
    Pixel::Type particleDrawType{Pixel::Type::Sand};
    unsigned int drawSize{1};
    // Constants calculated based on simulation controls
    unsigned int GridWidth =   9 * 5; // static_cast<unsigned int>(WindowWidth / PixelSize);
    unsigned int GridHeight =  9 * 5; // static_cast<unsigned int>(WindowHeight / PixelSize);
    unsigned int GridDepth =   9 * 5; // static_cast<unsigned int>(WindowDepth / PixelSize);


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

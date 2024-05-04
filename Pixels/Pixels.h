//
// Created by jonah on 5/2/2024.
//

#pragma once

#include "../Graphics.h"
#include "../Window.h"
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
    void Draw(Graphics &gfx) const;

    ~Pixels();
private:
    wrl::ComPtr<ID3D11Buffer> vertexBuffer;
    wrl::ComPtr<ID3D11Buffer> instanceBuffer;

    std::map<Position, Pixel*> pixels;
    float stepTime{0.1f};
};

//
// Created by jonah on 5/2/2024.
//

#pragma once

#include "../Graphics.h"
#include <map>

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

struct Position {
    float x;
    float y;
    auto operator<=>(Position const &) const = default;
};

class Pixel {
    friend class Pixels;
public:
    Pixel(const Color &color_): color(color_) {}
    ~Pixel();
private:
    const Color color;
};

class Pixels {
public:
    Pixels(Graphics &gfx);

    Pixels();

    void Update(float dt);
    void Draw(Graphics &gfx) const;

    ~Pixels();
private:
    std::map<Position, Pixel*> pixels;
    float stepTime{0.5f};
};

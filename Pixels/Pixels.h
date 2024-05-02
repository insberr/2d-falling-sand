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
};

class Pixel {
    friend class Pixels;
public:
    Pixel();
    ~Pixel();
private:
    Color color;
};

class Pixels {
public:
    Pixels(Graphics &gfx);

    Pixels();

    void Draw(Graphics &gfx);

    ~Pixels();
private:
    std::map<Position, Pixel> pixels;
};

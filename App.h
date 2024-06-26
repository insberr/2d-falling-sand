//
// Created by jonah on 5/1/2024.
//

#pragma once

#include "Window.h"
#include "EngineTimer.h"
#include "Pixels/Pixels.h"
#include "ImguiManager.h"
#include "Camera.h"

class App {
public:
    App();
    // Run the app
    int Run();
    ~App();
private:
    // sf_dt is the delta time with speed factor
    void Update(float dt, float sf_dt);
    void Render(float dt);
private:
    ImguiManager imgui;
    Window wnd;
    EngineTimer timer;
    // std::vector<std::unique_ptr<Drawable>> drawables;
    float speed_factor = 1.0f;
    Pixels pxs;
    Camera cam;
    // static constexpr size_t nDrawables = 100;
};
//
// Created by jonah on 4/2/2024.
//

#include "App.h"

#include <vector>
#include <memory>
#include <algorithm>
#include "Window.h"
#include "EngineTimer.h"
#include "Pixels/Pixels.h"

#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_win32.h"
#include "./imgui/imgui_impl_dx11.h"

//#include "Camera.h"


namespace dx = DirectX;
//GDIPlusManager gdipm;

App::App()
    :
    wnd( 1280,720,"Falling Sand Simulation" ),
    pxs{wnd.Gfx()}
{
    wnd.Gfx().SetProjection( DirectX::XMMatrixPerspectiveLH( 1.0f, 16.0f / 9.0f, 0.5f, 0.0f ) );
}

void App::Update(float dt) {
    pxs.Update(wnd, dt);
}

void App::Render(float dt)
{
    wnd.Gfx().BeginFrame(0.07f, 0.0f, 0.12f);
    // wnd.Gfx().SetCamera(cam.GetMatrix());
//    for (int x = 10; x < 800; x += 20) {
//        for (int y = 10; y < 600; y += 20) {
//            wnd.Gfx().DrawRect(x, y);
//        }
//    }


    pxs.Draw(wnd.Gfx());

    /* ImGui Stuff */
    if (ImGui::Begin("Simulation")) {
        // ImGui::SliderFloat("Speed Factor", &speed_factor, -5.0f, 5.0f);
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::Text("Delta Time %.3f ms", dt * 1000.0f);
        ImGui::Spacing();
        ImGui::Text("Controls");
        ImGui::Text("Left Mouse Button TO Draw");
        ImGui::Text("Right Mouse Button TO Erase");
    }
    ImGui::End();

    // Camera controls
    // cam.SpawnControlWindow();

    // Present the frame to the GPU
    wnd.Gfx().EndFrame();
}

App::~App() {}


int App::Run()
{
    while( true )
    {
        const float dt = timer.Mark() * speed_factor;
        // process all messages pending, but to not block for new messages
        if( const auto ecode = Window::ProcessMessages() )
        {
            // if return optional has value, means we're quitting so return exit code
            return *ecode;
        }

        Update(dt);
        Render(dt);


    }
}

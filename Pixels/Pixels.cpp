//
// Created by jonah on 5/2/2024.
//
// Credits to https://www.rastertek.com/dx11tut37.html For the example on instancing
//

#include <wrl.h>
#include "Pixels.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <cmath>
#include <iostream>

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_win32.h"
#include "../imgui/imgui_impl_dx11.h"

namespace dx = DirectX;

struct Vertex {
    struct {
        float x;
        float y;
        float z;
    } pos;
};

const float side = 0.5f;
const Vertex vertices[] = {
    //    { -0.5f,  0.5f, 0.0f, 255,   0,   0, 255 },
    //    {  0.5f,  0.5f, 0.0f,   0, 255,   0, 255 },
    //    { -0.5f, -0.5f, 0.0f,   0,   0, 255, 255 },
    //    {  0.5f, -0.5f, 0.0f, 255, 255, 255, 255 },

    // {-side,-side,-side, 255,   0,   0, 255 }, // 0
    // { side,-side,-side,   0, 255,   0, 255 }, // 1
    // { -side,side,-side,   0,   0, 255, 255 }, // 2
    // { side,side,-side,  255, 255, 255, 255 }, // 3
    // { -side,-side,side, 255,   0,   0, 255 }, // 4
    // { side,-side,side,    0, 255,   0, 255 }, // 5
    // { -side,side,side,    0,   0, 255, 255 }, // 6
    // { side,side,side,   255, 255, 255, 255 }, // 7

    {-side,-side,-side }, // 0
    { side,-side,-side }, // 1
    { -side,side,-side }, // 2
    { side,side,-side }, // 3
    { -side,-side,side }, // 4
    { side,-side,side }, // 5
    { -side,side,side }, // 6
    { side,side,side }, // 7

    // { -1.0f, -1.0f, -1.0f },
    // {  1.0f, -1.0f, -1.0f },
    // { -1.0f,  1.0f, -1.0f },
    // {  1.0f,  1.0f, -1.0f },
    // { -1.0f, -1.0f,  1.0f },
    // {  1.0f, -1.0f,  1.0f },
    // { -1.0f,  1.0f,  1.0f },
    // {  1.0f,  1.0f,  1.0f },
};

const unsigned short indices[] = {
    0,2,1, 2,3,1,
    1,3,5, 3,7,5,
    2,6,3, 3,6,7,
    4,5,7, 4,7,6,
    0,4,2, 2,4,6,
    0,1,4, 1,5,4
};

Pixels::Pixels(Graphics &gfx) {
//    // Random Number
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> range(static_cast<int>(Pixel::Type::Sand), static_cast<int>(Pixel::Type::Water)); // define the range

    for (int lx = 1; lx < 6; ++lx) {
        for (int ly = 10; ly < 15; ++ly) {
            for (int lz = 1; lz < 6; ++lz) {
                Position pos = {
                        lx, ly, lz
                };
                auto pixel = std::make_shared<Pixel>(static_cast<Pixel::Type>(range(gen)), pos);

                pixels.insert(std::pair<Position, std::shared_ptr<Pixel>>(pos, pixel));
            }
        }
    }

    // Insert debug
    pixels.insert(
        std::pair<Position, std::shared_ptr<Pixel>>(
            Position(0, 0, 0),
            std::make_shared<Pixel>(Pixel::Type::Debug, Position(0, 0, 0))
        )
    );

    // Create the vertex buffer
    D3D11_BUFFER_DESC bd = {};
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.CPUAccessFlags = 0u;
    bd.MiscFlags = 0u;
    bd.ByteWidth = sizeof(vertices);
    bd.StructureByteStride = sizeof(Vertex);

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices;

    gfx.device->CreateBuffer(&bd, &sd, &vertexBuffer);


    // Instance Buffer (Might want to update this every frame??)
    // auto instances = new PixelInstance[pixels.size()];
    // unsigned loopCount = 0;
    for (const auto& [pos, pix] : pixels) {
        pixelInstances.push_back(pix->GetInstance(pos));
    }
    D3D11_BUFFER_DESC instanceBufferDesc = {};
    instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.ByteWidth = sizeof(PixelInstance) * pixels.size();
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    instanceBufferDesc.MiscFlags = 0;
    instanceBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA instanceData = {};
    instanceData.pSysMem = pixelInstances.data(); // temp??
    instanceData.SysMemPitch = 0;
    instanceData.SysMemSlicePitch = 0;
    gfx.device->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);
    // delete[] instances;
    // instances = nullptr;


    // Create Index buffer
    wrl::ComPtr<ID3D11Buffer> indexBuffer;
    D3D11_BUFFER_DESC ibd = {};
    ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    ibd.Usage = D3D11_USAGE_DEFAULT;
    ibd.CPUAccessFlags = 0u;
    ibd.MiscFlags = 0u;
    ibd.ByteWidth = sizeof(indices);
    ibd.StructureByteStride = sizeof(unsigned short);
    D3D11_SUBRESOURCE_DATA isd = {};
    isd.pSysMem = indices;
    gfx.device->CreateBuffer(&ibd, &isd, &indexBuffer);

    gfx.context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);


    // Set World Transform Matrix Constant Buffer
    UpdateConstantBuffer(gfx);


    wrl::ComPtr<ID3DBlob> shaderBlob;

    /* Create Pixels Shader */
    // Load pixel shader file
    D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &shaderBlob, nullptr);

    wrl::ComPtr<ID3D11PixelShader> pixelShader;
    gfx.device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &pixelShader);
    // Bind the pixel shader
    gfx.context->PSSetShader(pixelShader.Get(), nullptr, 0);

    /* Create Vertex Shader */
    // Load vertex shader file
    D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &shaderBlob, nullptr);
    wrl::ComPtr<ID3D11VertexShader> vertexShader;
    gfx.device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &vertexShader);
    // Bind the vertex shader
    gfx.context->VSSetShader(vertexShader.Get(), nullptr, 0);

    // Input (vertex) layout (2d position only)
    wrl::ComPtr<ID3D11InputLayout> inputLayout;
    const D3D11_INPUT_ELEMENT_DESC ied[] = {
            { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            // { "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12u, D3D11_INPUT_PER_VERTEX_DATA, 0 },

            { "InstancePosition", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "InstanceColor", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 12u, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
    };
    // Create Layout
    gfx.device->CreateInputLayout(
            ied,
            (UINT)std::size(ied),
            shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(),
            &inputLayout
    );

    gfx.context->IASetInputLayout(inputLayout.Get());



    // Bind render target
    // Yeah dont do this here, we cause the depth thingy to stop working
    // gfx.context->OMSetRenderTargets(1u, gfx.target.GetAddressOf(), nullptr);

    // Set primitive topology to triangle list
    gfx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Configure Viewport
    // Already done
    // D3D11_VIEWPORT vp;
    // vp.Width = 1280;
    // vp.Height = 720;
    // vp.MinDepth = 0;
    // vp.MaxDepth = 1;
    // vp.TopLeftX = 0;
    // vp.TopLeftY = 0;
    // gfx.context->RSSetViewports(1u, &vp);
}

//struct ConstantBuffer {
//    dx::XMMATRIX transform;
//    struct Col {
//        float r;
//        float g;
//        float b;
//        float a;
//    } color;
//};

void Pixels::UpdateConstantBuffer(Graphics &gfx) {
    // Set World Transform Matrix Constant Buffer
    struct ConstantBuffer {
        dx::XMMATRIX transform;
    };
    const ConstantBuffer cb = {
        .transform = dx::XMMatrixTranspose(
            dx::XMMatrixScaling(PixelSize, PixelSize, PixelSize) *
            // dx::XMMatrixRotationY( 180.0f * (3.14159f / 180.0f )) *
            // dx::XMMatrixRotationZ(100.0f * (3.14159f / 180.0f ))
            gfx.GetCamera() *
            gfx.GetProjection()
        )
    };

    wrl::ComPtr<ID3D11Buffer> constantBuffer;

    D3D11_BUFFER_DESC cbd;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbd.Usage = D3D11_USAGE_DYNAMIC;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    cbd.MiscFlags = 0u;
    cbd.ByteWidth = sizeof(cb);
    cbd.StructureByteStride = 0u;
    D3D11_SUBRESOURCE_DATA csd = {};
    csd.pSysMem = &cb;
    gfx.device->CreateBuffer(&cbd, &csd, &constantBuffer);
    // Bind constant buffer to vertex shader
    gfx.context->VSSetConstantBuffers(0u, 1u, constantBuffer.GetAddressOf());
}

void Pixels::Draw(Graphics &gfx) {
    renderTime.Mark();

//    for (const auto& [pos, pixel] : pixels) {
//        // Create a constant buffer for our transformation matrix
//        // const float scale = 0.1f;
//        const float xpos = ((pos.x * PixelSize) + (PixelSize / 2)) / 400.0f - 1.0f;
//        const float ypos = -((pos.y * PixelSize) + (PixelSize / 2)) / 300.0f + 1.0f;
//
//        const auto colorForPix = ColorForPixel(*pixel);
//
//        const ConstantBuffer cb = {
//            .transform = dx::XMMatrixTranspose(
//                dx::XMMatrixScaling((PixelSize / 400.0f), (PixelSize / 300.0f), 1.0f) *
//                dx::XMMatrixTranslation(xpos, ypos, 0.0f)
//            ),
//            .color = {
//                colorForPix.r / 255.0f,
//                colorForPix.g / 255.0f,
//                colorForPix.b / 255.0f,
//                colorForPix.a / 255.0f
//            }
//        };
//        wrl::ComPtr<ID3D11Buffer> constantBuffer;
//        D3D11_BUFFER_DESC cbd;
//        cbd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
//        cbd.Usage = D3D11_USAGE_DYNAMIC;
//        cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
//        cbd.MiscFlags = 0u;
//        cbd.ByteWidth = sizeof(cb);
//        cbd.StructureByteStride = 0u;
//        D3D11_SUBRESOURCE_DATA csd = {};
//        csd.pSysMem = &cb;
//        gfx.device->CreateBuffer(&cbd, &csd, &constantBuffer);
//
//        // Bind constant buffer to vertex shader
//        gfx.context->VSSetConstantBuffers(0u, 1u, constantBuffer.GetAddressOf());
//
//        // Issue the draw command to draw the verticies
//        gfx.context->DrawIndexed((UINT)std::size(indices), 0u, 0u);
//    }


    // Instance Buffer (Might want to update this every frame??)
//    auto instances = new PixelInstance[pixels.size()];
//    unsigned loopCount = 0;
//    for (const auto& [pos, pix] : pixels) {
//        const auto color = pix->GetColor();
//
//        instances[loopCount] = {
//                .worldPosition {
//                        -(static_cast<float>(pos.x) - (static_cast<float>(GridWidth) / 2.0f) + 0.5f),
//                        static_cast<float>(pos.y) - (static_cast<float>(GridHeight) / 2.0f) + 0.5f
//                },
//                .color {
//                        static_cast<float>(color.r) / 255.0f,
//                        static_cast<float>(color.g) / 255.0f,
//                        static_cast<float>(color.b) / 255.0f,
//                        static_cast<float>(color.a) / 255.0f
//                }
//        };
//        ++loopCount;
//    }

    UpdateConstantBuffer(gfx);

    D3D11_BUFFER_DESC instanceBufferDesc = {};
    instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.ByteWidth = sizeof(PixelInstance) * pixelInstances.size();
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    instanceBufferDesc.MiscFlags = 0;
    instanceBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA instanceData = {};
    instanceData.pSysMem = pixelInstances.data(); // temp??
    instanceData.SysMemPitch = 0;
    instanceData.SysMemSlicePitch = 0;
    gfx.device->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);
    // delete[] instances;
    // instances = nullptr;
    // pixelInstances.clear();
    // pixelInstances.reserve(pixels.size());

    // const UINT stride = sizeof(Vertex);
    unsigned int strides[2] = {
        sizeof(Vertex),
        sizeof(PixelInstance)
    };

    // const UINT offset = 0u;
    unsigned int offsets[2] = { 0, 0 };

    ID3D11Buffer* bufferPointers[2] = {
        vertexBuffer.Get(),
        instanceBuffer.Get()
    };

    // Set buffer to pipeline
    gfx.context->IASetVertexBuffers(0, 2u, bufferPointers, strides, offsets);

    gfx.context->DrawIndexedInstanced((UINT)std::size(indices), pixels.size(), 0u, 0u, 0u);

    timeTakenRender = renderTime.Mark();
    DrawUI(gfx);
}

Pixels::~Pixels() {

}

std::pair<int, int> lastMousePos = std::pair(0, 0);
// Random Number
static std::random_device rd; // obtain a random number from hardware
static std::mt19937 gen(rd()); // seed the generator
static std::uniform_int_distribution<> range(-1, 1); // define the range
static std::uniform_int_distribution<> shouldSidewaysMove(0, 1); // define the range

void Pixels::Update(Window &wnd, Camera &cam, float dt) {
    // Used for tracking the time it takes to run
    updateTime.Mark();

    if (dt == 0.0f) {
        timeTakenUpdate = updateTime.Mark();
        return;
    }

    pixelInstances.clear();
    pixelInstances.reserve(pixels.size());

    // Do particle drawing/erasing
    Update_Drawing(wnd, cam);

    // Create a copy of the pixels so we can modify the pixels data without breaking the loop
    std::map<Position, std::shared_ptr<Pixel>> newPixels = pixels;

    // for (auto iter = newPixels.rbegin(); iter != newPixels.rend(); ++iter) {
        // I want the y positions in reverse order to prevent the weird stalled movement
        // const auto& [pos, pix] = *iter;
    for (const auto& [pos, pix] : newPixels) {
        // Completely ignore debug particles
        if (pix->GetType() == Pixel::Type::Debug) {
            pixelInstances.push_back(pix->GetInstance(pos));
            continue;
        }

        vec3 realPos = pix->realPosition;
        vec3 newRealPos = pix->realPosition;

        // First check if pixel is out of bounds of the grid
        if (
            (realPos.y < 0.0f || realPos.x < 0.0f || realPos.z < 0.0f) ||
            (realPos.y > static_cast<float>(GridHeight) || realPos.x > static_cast<float>(GridWidth) || realPos.z > static_cast<float>(GridDepth))
        ) {
            pixels.erase(pos);
            continue;
        }

        // Set new real pos to where we want to go
        newRealPos.y -= pix->Velocity().y * dt;

        // Limit pixels from going off the bottom of the grid
        if (newRealPos.y <= 0.0f) {
            // If the "floor" setting is enabled, stop the particle from moving
            if (BottomStop) {
                pixelInstances.push_back(pix->GetInstance(pos));
                continue;
            }
            // Else move back to the top of the grid

            newRealPos.y = static_cast<float>(GridHeight);
        }

        // X and Z movement
        if (shouldSidewaysMove(gen)) {
            // clamp this value so that sand doesnt fall off the sides of the world
            const float clamp = std::clamp<float>(newRealPos.x + (range(gen)), 0, GridWidth);
            newRealPos.x = clamp;
        }
        if (shouldSidewaysMove(gen)) {
            // clamp this value so that sand doesnt fall off the sides of the world
            const float clamp = std::clamp<float>(newRealPos.z + (range(gen)), 0, GridDepth); // min(max(2, newRealPos.z + range(gen)), GridWidth - 2);
            newRealPos.z = clamp;
        }

        /* All position updates need to happen before the following */
        // Create the Grid Position for new real position
        vec3 flooredNewRealPos = newRealPos.floor();
        Position newGridPos(
            flooredNewRealPos.x,
            flooredNewRealPos.y,
            flooredNewRealPos.z
        );

        // if flooredRealPos == pos, accumulate and continue
        if (
            flooredNewRealPos.y == pos.y &&
            flooredNewRealPos.x == pos.x &&
            flooredNewRealPos.z == pos.z
        ) {
            pix->realPosition = newRealPos;
            pixelInstances.push_back(pix->GetInstance(pos));
            continue;
        }

        // Check to see if we can move there
        if (pixels.contains(newGridPos)) {
            // pixels.insert(std::pair(pos, pix));

            // Need to set the pixel's real pos to the position it can go given the new real pos
            // Doing this removed the weird "scan" effect that is kind of jarring
            // find out how to make the movement all uniform
            // *** This is not needed now that we loop in reverse
//            pix->realPosition = vec3(
//                pos.x,
//                pos.y,
//                pos.z
//            );

            // Push to instances
            pixelInstances.push_back(pix->GetInstance(pos));
            continue;
        }

        // We can move so do
        // Set the pixel real pos
        pix->realPosition = newRealPos;

        auto nodePix = pixels.extract(pos);
        // Set the new grid pos
        nodePix.key() = newGridPos;
        pixels.insert(std::move(nodePix));
        // Push to instances
        pixelInstances.push_back(pix->GetInstance(newGridPos));

//        Position newPos = pos;
//
//        if (pos.y >= GridHeight - 1) {
//            newPos.y = 0;
//            if (BottomStop) {
//                pixelInstances.push_back(pix->GetInstance(pos, GridWidth, GridHeight, GridDepth));
//                continue;
//            }
//        } else {
//            newPos.y += pix->Velocity().y * dt;
//        }
//
//        if (shouldSidewaysMove(gen)) {
//            // clamp this value so that sand doesnt fall off the sides of the world
//            float clamp = min(max(2, newPos.x + range(gen)), GridWidth - 2);
//            newPos.x = clamp * dt;
//        }
//
//
//        bool exists = pixels.contains(newPos);
//        if (exists) {
//            // pixels.insert(std::pair(pos, pix));
//            pixelInstances.push_back(pix->GetInstance(pos, GridWidth, GridHeight, GridDepth));
//            continue;
//        }
//
//        auto nodePix = pixels.extract(pos);
//        nodePix.key() = newPos;
//        pixels.insert(std::move(nodePix));
//        pixelInstances.push_back(pix->GetInstance(newPos, GridWidth, GridHeight, GridDepth));

    }

    // std::ranges::reverse(pixelInstances.begin(), pixelInstances.end());
    timeTakenUpdate = updateTime.Mark();
}

static bool drawing = false;
void Pixels::Update_Drawing(Window& wnd, Camera& cam) {
    // Prevent drawing if disabled
    if (not drawingEnabled) return;

    // Calculate the position the player is looking at
    const auto lookVector = cam.GetLookVector();
    const auto cameraPos = cam.GetPos();
    vec3 look(lookVector.x, lookVector.y, lookVector.z);
    vec3 camPos(cameraPos.x, cameraPos.y, cameraPos.z);
    vec3 lookScaled = look.normalize() * drawDistance;
    vec3 drawPos = camPos + lookScaled;
    drawPos = drawPos.floor();
    Position tempPos(drawPos.x, drawPos.y, drawPos.z);
    Pixel tempPixel(Pixel::Type::Debug, tempPos);
    // Add a debug particle at the pos the player is looking at
    pixelInstances.push_back(tempPixel.GetInstance(tempPos));

    // Return if neither are down
    if (not wnd.mouse.LeftIsPressed() && not wnd.mouse.RightIsPressed()) {
        // We are not drawing
        drawing = false;
        return;
    }
    if (ImGui::IsAnyItemActive()) {
        // We are not drawing
        drawing = false;
        return;
    }

    // If was not drawing last update,
    // - Set the last mouse pos to current.
    // - Set drawing variable to true
    // - Return. Because ImGui::IsAnyActive does not become active till the next frame
    //     and we don't want to draw yet because we don't know if the interaction is with imgui
    if (not drawing) {
        lastMousePos = wnd.mouse.GetPos();
        drawing = true;
        return;
    }

    if (
        (drawPos.y > 0.0f && drawPos.x > 0.0f && drawPos.z > 0.0f) &&
        (drawPos.y < static_cast<float>(GridHeight) && drawPos.x < static_cast<float>(GridWidth) && drawPos.z < static_cast<float>(GridDepth))
    ) {
        Position pos(
                std::floorf(drawPos.x / PixelSize),
                std::floorf(drawPos.y / PixelSize),
                std::floorf(drawPos.z / PixelSize)
        );
        const int thickness = drawSize;
        for (int dx = -thickness; dx <= thickness; ++dx) {
            for (int dy = -thickness; dy <= thickness; ++dy) {
                for (int dz = -thickness; dz <= thickness; ++dz) {
                    Position posDraw(
                            pos.x + dx,
                            pos.y + dy,
                            pos.z + dz
                    );

                    if (wnd.mouse.LeftIsPressed()) {
                        pixels.insert_or_assign(posDraw, std::make_shared<Pixel>(particleDrawType, posDraw));
                    } else {
                        size_t removed = pixels.erase(posDraw);
                    }
                }
            }
        }
    }
}

void Pixels::DrawUI(Graphics &gfx) {
    // ImGui::ShowDemoWindow();

    if (ImGui::Begin("Simulation Controls")) {
        // int pixelSizeInt = static_cast<int>(PixelSize);
        if (ImGui::SliderFloat("Particle Size", &PixelSize, 0.001f, 10.0f, "%.3f")) {
            // PixelSize = static_cast<float>(pixelSizeInt);

            // Update the constant buffer matrix
            UpdateConstantBuffer(gfx);
        }

        ImGui::SliderInt("Grid Size X", (int*)&GridWidth, 1, 50);
        ImGui::SliderInt("Grid Size Y", (int*)&GridHeight, 1, 50);
        ImGui::SliderInt("Grid Size Z", (int*)&GridDepth, 1, 50);

        ImGui::Spacing();

        ImGui::Checkbox("Floor", &BottomStop);
        ImGui::Checkbox("Enable Drawing", &drawingEnabled);
        ImGui::SliderInt("Draw Type", (int*)&particleDrawType, 1, (int)Pixel::Type::last - 1);
        ImGui::SliderInt("Draw Size", (int*)&drawSize, 0, 10);
        ImGui::SliderFloat("Draw Distance", &drawDistance, 0, 20, "%.f");
    }
    ImGui::End();
    
    if (ImGui::Begin("Simulation Stats")) {
        ImGui::Text("Number Of Particles: %i", pixels.size());
        ImGui::Text("Update Time: %.5f ms", timeTakenUpdate * 1000.0f);
        ImGui::Text("Render Time: %.5f ms", timeTakenRender * 1000.0f);

        for (const auto&[name, time] : updateTimings) {
            ImGui::Text(name.c_str(), time);
        }
        updateTimings.clear();
    }
    ImGui::End();
}

NormalizedColor Color::normalize() const {
    return {
        static_cast<float>(r) / 255.0f,
        static_cast<float>(g) / 255.0f,
        static_cast<float>(b) / 255.0f,
        static_cast<float>(a) / 255.0f
    };
}

Pixel::Pixel(Type type_, Position pos) : realPosition(vec3(pos.x, pos.y, pos.z)), type(type_) {
}

PixelInstance Pixel::GetInstance(const Position& pos) {
    const auto color = GetColor();

    PixelInstance inst = {
        .worldPosition {
            // We floor them because they render on a grid, but their positions are controlled by physics
            pos.x, // - (static_cast<float>(GridWidth) / 2.0f) + 0.5f,
            pos.y, // - (static_cast<float>(GridHeight) / 2.0f) + 0.5f,
            pos.z// - (static_cast<float>(GridDepth) / 2.0f) + 0.5f
        },
        .color = color.normalize()
    };
    return inst;
}

Color Pixel::GetColor() const {
    switch (type) {
        case Pixel::Type::Unknown:
            return { 250, 0, 250, 255 };
        case Pixel::Type::Debug:
            return { 250, 0, 0, 255 };
        case Pixel::Type::Sand:
            // I got the color right on point first try!
            return { 200, 150, 10, 255 };
        case Pixel::Type::Water:
            return { 13, 136, 188, 255 };
        case Pixel::Type::Steam:
            return { 255, 255, 255, 150 };
        case Pixel::Type::Lava:
            return { 239, 103, 23, 255 };
        case Pixel::Type::Rock:
            return { 170, 170, 170, 255 };
        default:
            return { 250, 0, 250, 255 };
    }
}

vec3 Pixel::Velocity() const {
    switch (type) {
        case Pixel::Type::Unknown:
        case Pixel::Type::Debug:
            return vec3 { 0, 0, 0 };
        case Pixel::Type::Sand:
        case Pixel::Type::Water:
            return vec3 { 0, 9.8f, 0};
        case Pixel::Type::Steam:
            return vec3 { 0, -1.0f, 0 };
        case Pixel::Type::Lava:
            return vec3 { 0, 1.0f, 0};
        case Pixel::Type::Rock:
            return vec3 { 0, 9.8f * 2.0f, 0};
        default:
            return vec3 { 0, 1.0f, 0};
    }
}

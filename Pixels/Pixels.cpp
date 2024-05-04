//
// Created by jonah on 5/2/2024.
//

#include <wrl.h>
#include "Pixels.h"
#include <d3d11.h>
#include <DirectXMath.h>
#include <iostream>
#include <cmath>

namespace dx = DirectX;

const float PixelSize = 1.0f;
const float WindowWidth = 800.0f;
const float WindowHeight = 600.0f;
const unsigned int GridWidth = static_cast<unsigned int>(WindowWidth / PixelSize);
const unsigned int GridHeight = static_cast<unsigned int>(WindowHeight / PixelSize);
const bool BottomStop = true;

Color ColorForPixel(const Pixel& pixel) {
    const auto type = pixel.GetType();
    switch (type) {
        case Pixel::Type::Unknown:
            return { 250, 0, 250, 255 };
        case Pixel::Type::Sand:
            // I got the color right on point first try!
            return { 200, 150, 10, 255 };
        case Pixel::Type::Water:
            return { 13, 136, 188, 255 };
        case Pixel::Type::Lava:
            return { 239, 103, 23, 255 };
        case Pixel::Type::Rock:
            return { 170, 170, 170, 255 };
        default:
            return { 250, 0, 250, 255 };
    }
}

struct Vertex {
    struct {
        float x;
        float y;
    } pos;
    Color color;
};

const Vertex vertices[] = {
        { -0.5f,  0.5f, 255,   0,   0, 255 },
        {  0.5f,  0.5f,   0, 255,   0, 255 },
        { -0.5f, -0.5f,   0,   0, 255, 255 },
        {  0.5f, -0.5f, 255, 255, 255, 255 },
};

const unsigned short indices[] = {
        0, 1, 2,
        2, 1, 3
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

Pixels::Pixels(Graphics &gfx) {
//    // Random Number
//    std::random_device rd; // obtain a random number from hardware
//    std::mt19937 gen(rd()); // seed the generator
//    std::uniform_int_distribution<> range(1, static_cast<int>(Pixel::Type::last) - 1); // define the range
//
//    int lx = 0; // 10.0f
//    int ly = 0;
//    Color lc = { 0, 255, 0, 1 };

//    for (unsigned int i = 0; i < GridWidth * GridHeight; ++i) {
//        auto pixel = new Pixel(static_cast<Pixel::Type>(range(gen)));
//        Position pos = { lx, ly };
//
//
//        pixels.insert(std::pair<Position, Pixel*>(pos, pixel));
//
//        lx += 1;
//        if (lx >= GridWidth) {
//            ly += 1;
//            lx = 0;
//        }
//
//        ++lc.r;
//        --lc.g;
//    }

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
    auto instances = new PixelInstance[pixels.size()];
    unsigned loopCount = 0;
    for (const auto& [pos, pix] : pixels) {
        const auto color = ColorForPixel(*pix);

        instances[loopCount] = {
            .worldPosition {
                -(static_cast<float>(pos.x) - (static_cast<float>(GridWidth) / 2.0f) + 0.5f),
                static_cast<float>(pos.y) - (static_cast<float>(GridHeight) / 2.0f) + 0.5f
            },
            .color {
                static_cast<float>(color.r) / 255.0f,
                static_cast<float>(color.g) / 255.0f,
                static_cast<float>(color.b) / 255.0f,
                static_cast<float>(color.a) / 255.0f
            }
        };
        ++loopCount;
    }
    D3D11_BUFFER_DESC instanceBufferDesc = {};
    instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.ByteWidth = sizeof(PixelInstance) * pixels.size();
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    instanceBufferDesc.MiscFlags = 0;
    instanceBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA instanceData = {};
    instanceData.pSysMem = instances; // temp??
    instanceData.SysMemPitch = 0;
    instanceData.SysMemSlicePitch = 0;
    gfx.device->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);
    delete[] instances;
    instances = nullptr;


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
            { "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8u, D3D11_INPUT_PER_VERTEX_DATA, 0 },

            { "InstancePosition", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
            { "InstanceColor", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 8u, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
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



    // Set World Transform Matrix Constant Buffer
    struct ConstantBuffer {
        dx::XMMATRIX transform;
    };
    const ConstantBuffer cb = {
        .transform = dx::XMMatrixTranspose(
            dx::XMMatrixScaling(PixelSize / 400.0f, PixelSize / 300.0f, 1.0f) *
            dx::XMMatrixRotationZ( 180.0f * (3.14159f / 180.0f ))
//            dx::XMMatrixTranslation(
//                // -(1.0f - ( 1.0f / (PixelSize * 2.0f) )),
//                -1.0f + (0.5f / PixelSize),
//                -1.0f + (0.5f / PixelSize),
//                0.0f
//            )
        )
    };
    wrl::ComPtr<ID3D11Buffer> constantBuffer;
    D3D11_BUFFER_DESC cbd;
    cbd.BindFlags = D3D10_BIND_CONSTANT_BUFFER;
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


    // Bind render target
    gfx.context->OMSetRenderTargets(1u, gfx.target.GetAddressOf(), nullptr);

    // Set primitive topology to triangle list
    gfx.context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Configure Viewport
    D3D11_VIEWPORT vp;
    vp.Width = 800;
    vp.Height = 600;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    gfx.context->RSSetViewports(1u, &vp);
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

void Pixels::Draw(Graphics &gfx) {

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
    auto instances = new PixelInstance[pixels.size()];
    unsigned loopCount = 0;
    for (const auto& [pos, pix] : pixels) {
        const auto color = ColorForPixel(*pix);

        instances[loopCount] = {
                .worldPosition {
                        -(static_cast<float>(pos.x) - (static_cast<float>(GridWidth) / 2.0f) + 0.5f),
                        static_cast<float>(pos.y) - (static_cast<float>(GridHeight) / 2.0f) + 0.5f
                },
                .color {
                        static_cast<float>(color.r) / 255.0f,
                        static_cast<float>(color.g) / 255.0f,
                        static_cast<float>(color.b) / 255.0f,
                        static_cast<float>(color.a) / 255.0f
                }
        };
        ++loopCount;
    }
    D3D11_BUFFER_DESC instanceBufferDesc = {};
    instanceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    instanceBufferDesc.ByteWidth = sizeof(PixelInstance) * pixels.size();
    instanceBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    instanceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    instanceBufferDesc.MiscFlags = 0;
    instanceBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA instanceData = {};
    instanceData.pSysMem = instances; // temp??
    instanceData.SysMemPitch = 0;
    instanceData.SysMemSlicePitch = 0;
    gfx.device->CreateBuffer(&instanceBufferDesc, &instanceData, &instanceBuffer);
    delete[] instances;
    instances = nullptr;

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
}

Pixels::~Pixels() {

}

std::pair<int, int> lastMousePos = std::pair(0, 0);

void Pixels::Update(Window &wnd, float dt) {
    // if (dt == 0.0f) return;


    if (wnd.mouse.LeftIsPressed()) {
        const auto [lmx, lmy] = lastMousePos;
        const auto [mouseX, mouseY] = wnd.mouse.GetPos();

        // const int gridPosX = mouseX / static_cast<int>(PixelSize);
        // const int gridPosY = mouseY / static_cast<int>(PixelSize);

//        pixels.insert_or_assign({ gridPosX, gridPosY }, new Pixel(Pixel::Type::Sand));

        // Define the thickness of the drawing
        int thickness = 0; // You can adjust this value to increase or decrease the thickness

        float diffMX = mouseX - lmx;
        float diffMY = mouseY - lmy;
        float distance = std::sqrtf(diffMX * diffMX + diffMY * diffMY);

        int numSteps = static_cast<int>(distance / 1.0f);
        if (numSteps < 1) numSteps = 1;

        // Perform interpolation and update pixels along the path
        for (int i = 0; i <= numSteps; ++i) {
            const float interpolateX = lmx + (mouseX - lmx) * (static_cast<float>(i) / numSteps);
            const float interpolateY = lmy + (mouseY - lmy) * (static_cast<float>(i) / numSteps);

            const float gridPosX = interpolateX / PixelSize;
            const float gridPosY = interpolateY / PixelSize;

            if (interpolateX >= 0.0f && interpolateY >= 0.0f) {
                // Update the pixels in a square around the current position to make the drawing thicker
                for (int dx = -thickness; dx <= thickness; ++dx) {
                    for (int dy = -thickness; dy <= thickness; ++dy) {
                        // where to draw
                        int x = static_cast<int>(gridPosX) + dx;
                        int y = static_cast<int>(gridPosY) + dy;
                        pixels.insert_or_assign({ x, y }, new Pixel(Pixel::Type::Sand));
                    }
                }
            }
        }

        lastMousePos = std::pair(mouseX, mouseY);
//        const int gridPosX = mouseX / static_cast<int>(PixelSize);
//        const int gridPosY = mouseY / static_cast<int>(PixelSize);
//
////        const float pixPosX = (gridPosX * 20.0f) + 10.0f;
////        const float pixPosY = (gridPosY * 20.0f) + 10.0f;
//
//        // size_t removed = pixels.erase({ pixPosX, pixPosY });
//        pixels.insert_or_assign({ gridPosX, gridPosY }, new Pixel(Pixel::Type::Sand));


        // std::cout << pixPosX << " " << pixPosY << " " << removed << std::endl;
    }
    if (wnd.mouse.RightIsPressed()) {
        const auto [mouseX, mouseY] = wnd.mouse.GetPos();

        const int gridPosX = mouseX / static_cast<int>(PixelSize);
        const int gridPosY = mouseY / static_cast<int>(PixelSize);

        // const float pixPosX = (gridPosX * 20.0f) + 10.0f;
        // const float pixPosY = (gridPosY * 20.0f) + 10.0f;

        size_t removed = pixels.erase({ gridPosX, gridPosY });
        // pixels.insert_or_assign({ pixPosX, pixPosY }, new Pixel(Pixel::Type::Sand));

        // std::cout << pixPosX << " " << pixPosY << " " << removed << std::endl;
    }


    stepTime -= dt;
    if (stepTime <= 0.0f) {
        stepTime = stepSpeed;
    } else {
        return;
    }

    // Random Number
    std::random_device rd; // obtain a random number from hardware
    std::mt19937 gen(rd()); // seed the generator
    std::uniform_int_distribution<> range(-1, 1); // define the range

    std::map<Position, Pixel*> newPixels = pixels;

    for (const auto& [pos, pix] : newPixels) {
        // Limit pixels from going off the bottom of the screen
        // later we put pixel at top of screen
        Position newPos = pos;

        if (pos.y >= GridHeight - 1) {
            newPos.y = 0;
            if (BottomStop) {
                continue;
            }
        } else {
            newPos.y += 1;
        }

        newPos.x += range(gen);

        bool exists = pixels.contains(newPos);
        if (exists) {
            pixels.insert(std::pair(pos, pix));
            continue;
        }

        auto nodePix = pixels.extract(pos);
        nodePix.key() = newPos;
        pixels.insert(std::move(nodePix));

        // auto [_, worked] = pixels.try_emplace(newPos, pix);

    }
}

//
// Created by jonah on 5/2/2024.
//

#include <wrl.h>
#include "Pixels.h"
#include <d3d11.h>
#include <DirectXMath.h>

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

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

Pixels::Pixels(Graphics &gfx) {
    float lx = 10.0f;
    float ly = 10.0f;
    Color lc = { 0, 255, 0, 1 };
    for (unsigned int i = 0; i < 1300; ++i) {
        auto pixel = new Pixel({ lc.r, lc.g, lc.b, 255 });
        Position pos = { lx, ly };

        pixels.insert(std::pair<Position, Pixel*>(pos, pixel));

        lx += 20.0f;
        if (lx >= 800) {
            ly += 40.0f;
            lx = 10.0f;
        }

        ++lc.r;
        --lc.g;
    }

    // Create the vertex buffer
    wrl::ComPtr<ID3D11Buffer> vertexBuffer;
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

    const UINT stride = sizeof(Vertex);
    const UINT offset = 0u;
    // Set buffer to pipeline
    gfx.context->IASetVertexBuffers(0, 1u, vertexBuffer.GetAddressOf(), &stride, &offset);

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
            { "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8u, D3D11_INPUT_PER_VERTEX_DATA, 0 }
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

struct ConstantBuffer {
    dx::XMMATRIX transform;
    struct Col {
        float r;
        float g;
        float b;
        float a;
    } color;
};

void Pixels::Draw(Graphics &gfx) const {
    for (const auto& [pos, pixel] : pixels) {
        // Create a constant buffer for our transformation matrix
        // const float scale = 0.1f;
        const float size = 20.0f;
        const float xpos = pos.x / 400.0f - 1.0f;
        const float ypos = -pos.y / 300.0f + 1.0f;

        const ConstantBuffer cb = {
            .transform = dx::XMMatrixTranspose(
                dx::XMMatrixScaling((size / 400.0f), (size / 300.0f), 1.0f) *
                dx::XMMatrixTranslation(xpos, ypos, 0.0f)
            ),
            .color = {
                pixel->color.r / 255.0f,
                pixel->color.g / 255.0f,
                pixel->color.b / 255.0f,
                pixel->color.a / 255.0f
            }
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

        // Issue the draw command to draw the verticies
        gfx.context->DrawIndexed((UINT)std::size(indices), 0u, 0u);
    }
}

Pixels::~Pixels() {

}

void Pixels::Update(float dt) {
    // if (dt == 0.0f) return;
    stepTime -= dt;
    if (stepTime <= 0.0f) {
        stepTime = 0.5f;
    } else {
        return;
    }

    for (const auto& [pos, pix] : pixels) {
        // Limit pixels from going off the bottom of the screen
        // later we put pixel at top of screen
        if (pos.y >= 500.0f) continue;

        Position newPos = pos;
        newPos.y += 20.0f;

        bool exists = pixels.contains(newPos);
        if (exists) continue;

        auto nodePix = pixels.extract(pos);
        nodePix.key() = newPos;
        pixels.insert(std::move(nodePix));

        // auto [_, worked] = pixels.try_emplace(newPos, pix);

    }
}

#include "Graphics.h"
#include "ErrorHandling/dxerr.h"
#include <sstream>
#include <d3dcompiler.h>
#include <cmath>
#include <DirectXMath.h>
#include "ErrorHandling/GraphicsThrowMacros.h"
#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_win32.h"
#include "./imgui/imgui_impl_dx11.h"

namespace wrl = Microsoft::WRL;
namespace dx = DirectX;

#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"D3DCompiler.lib")


Graphics::Graphics( HWND hWnd )
{
	DXGI_SWAP_CHAIN_DESC sd = {};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// for checking results of d3d functions
	HRESULT hr;

	// create device and front/back buffers, and swap chain and rendering context
	GFX_THROW_INFO( D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		swapCreateFlags,
		nullptr,
		0,
		D3D11_SDK_VERSION,
		&sd,
		&swap,
		&device,
		nullptr,
		&context
	) );

	// gain access to texture subresource in swap chain (back buffer)
	wrl::ComPtr<ID3D11Resource> pBackBuffer;
	GFX_THROW_INFO( swap->GetBuffer( 0,__uuidof(ID3D11Resource),&pBackBuffer ) );
	GFX_THROW_INFO( device->CreateRenderTargetView( pBackBuffer.Get(),nullptr,&target ) );
	
	// create depth stensil state
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	wrl::ComPtr<ID3D11DepthStencilState> pDSState;
	GFX_THROW_INFO( device->CreateDepthStencilState( &dsDesc,&pDSState ) );

	// bind depth state
	context->OMSetDepthStencilState( pDSState.Get(),1u );

	// create depth stensil texture
	wrl::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = 800u;
	descDepth.Height = 600u;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	GFX_THROW_INFO( device->CreateTexture2D( &descDepth,nullptr,&pDepthStencil ) );

	// create view of depth stensil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	GFX_THROW_INFO( device->CreateDepthStencilView(
		pDepthStencil.Get(),&descDSV,&DSV
	) );

	// bind depth stensil view to OM
	context->OMSetRenderTargets( 1u,target.GetAddressOf(),DSV.Get() );
	   
	// configure viewport
	D3D11_VIEWPORT vp;
	vp.Width = 800.0f;
	vp.Height = 600.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	context->RSSetViewports( 1u,&vp );

     ImGui_ImplDX11_Init(device.Get(), context.Get());
}

void Graphics::EndFrame()
{
    // imgui frame end
    if (imguiEnabled) {
        ImGui::Render();
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    }

	HRESULT hr;
#ifndef NDEBUG
	infoManager.Set();
#endif
	if( FAILED( hr = swap->Present( 1u,0u ) ) )
	{
		if( hr == DXGI_ERROR_DEVICE_REMOVED )
		{
			throw GFX_DEVICE_REMOVED_EXCEPT( device->GetDeviceRemovedReason() );
		}
		else
		{
			throw GFX_EXCEPT( hr );
		}
	}
}

//void Graphics::ClearBuffer( float red,float green,float blue ) noexcept
//{
//	const float color[] = { red,green,blue,1.0f };
//	pContext->ClearRenderTargetView( pTarget.Get(),color );
//	pContext->ClearDepthStencilView( pDSV.Get(),D3D11_CLEAR_DEPTH,1.0f,0u );
//}

void Graphics::DrawIndexed( UINT count ) noexcept(!IS_DEBUG)
{
	GFX_THROW_INFO_ONLY( context->DrawIndexed( count,0u,0u ) );
}

void Graphics::SetProjection( DirectX::FXMMATRIX proj ) noexcept
{
	projection = proj;
}

DirectX::XMMATRIX Graphics::GetProjection() const noexcept
{
	return projection;
}

void Graphics::BeginFrame(float red, float green, float blue) noexcept {
    if (imguiEnabled) {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }

    // Clear buffer
    const float color[] = { red,green,blue,1.0f };
    context->ClearRenderTargetView( target.Get(),color );
    context->ClearDepthStencilView( DSV.Get(),D3D11_CLEAR_DEPTH,1.0f,0u );
}

void Graphics::EnableImgui() noexcept {
    imguiEnabled = true;
}

void Graphics::DisableImgui() noexcept {
    imguiEnabled = false;
}

bool Graphics::IsImguiEnabled() const noexcept {
    return imguiEnabled;
}

void Graphics::SetCamera(DirectX::FXMMATRIX cam) noexcept {
    camera = cam;
}

DirectX::XMMATRIX Graphics::GetCamera() const noexcept {
    return camera;
}

Graphics::~Graphics() {
     ImGui_ImplDX11_Shutdown();
}

static bool once = false;
void Graphics::DrawRect(float posx, float posy) {
    HRESULT hr;

    struct Vertex {
        struct {
            float x;
            float y;
        } pos;
        struct {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
        } color;
    };

    const Vertex vertices[] = {
        { -0.5f,  0.5f, 255,   0,   0, 255 },
        {  0.5f,  0.5f,   0, 255,   0, 255 },
        { -0.5f, -0.5f,   0,   0, 255, 255 },
        {  0.5f, -0.5f, 255, 255, 255, 255 },
    };


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

        device->CreateBuffer(&bd, &sd, &vertexBuffer);

        const UINT stride = sizeof(Vertex);
        const UINT offset = 0u;
        // Set buffer to pipeline
        context->IASetVertexBuffers(0, 1u, vertexBuffer.GetAddressOf(), &stride, &offset);

    // Create Index buffer
    const unsigned short indices[] = {
            0, 1, 2,
            2, 1, 3
    };
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
    device->CreateBuffer(&ibd, &isd, &indexBuffer);

    context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);


    // Create a constant buffer for our transformation matrix
    struct ConstantBuffer {
        dx::XMMATRIX transform;
    };

    // const float scale = 0.1f;
    const float size = 20.0f;
    const float xpos = posx / 400.0f - 1.0f;
    const float ypos = -posy / 300.0f + 1.0f;

    const ConstantBuffer cb = {
            dx::XMMatrixTranspose(
                dx::XMMatrixScaling((size/400.0f), (size/300.0f), 1.0f) *
                dx::XMMatrixTranslation(xpos, ypos, 0.0f)
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
    device->CreateBuffer(&cbd, &csd, &constantBuffer);

    // Bind constant buffer to vertex shader
    context->VSSetConstantBuffers(0u, 1u, constantBuffer.GetAddressOf());




    wrl::ComPtr<ID3DBlob> shaderBlob;

    /* Create Pixels Shader */
    // Load pixel shader file
    D3DCompileFromFile(L"PixelShader.hlsl", nullptr, nullptr, "main", "ps_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &shaderBlob, nullptr);

    wrl::ComPtr<ID3D11PixelShader> pixelShader;
    device->CreatePixelShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &pixelShader);
    // Bind the pixel shader
    context->PSSetShader(pixelShader.Get(), nullptr, 0);

    /* Create Vertex Shader */
    // Load vertex shader file
    D3DCompileFromFile(L"VertexShader.hlsl", nullptr, nullptr, "main", "vs_5_0", D3DCOMPILE_ENABLE_STRICTNESS, 0, &shaderBlob, nullptr);
    wrl::ComPtr<ID3D11VertexShader> vertexShader;
    device->CreateVertexShader(shaderBlob->GetBufferPointer(), shaderBlob->GetBufferSize(), nullptr, &vertexShader);
    // Bind the vertex shader
    context->VSSetShader(vertexShader.Get(), nullptr, 0);

    // Input (vertex) layout (2d position only)
    wrl::ComPtr<ID3D11InputLayout> inputLayout;
    const D3D11_INPUT_ELEMENT_DESC ied[] = {
            { "Position", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
            { "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 8u, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    // Create Layout
    device->CreateInputLayout(
            ied,
            (UINT)std::size(ied),
            shaderBlob->GetBufferPointer(),
            shaderBlob->GetBufferSize(),
            &inputLayout
    );

    context->IASetInputLayout(inputLayout.Get());

    // Bind render target
    context->OMSetRenderTargets(1u, target.GetAddressOf(), nullptr);

    // Set primitive topology to triangle list
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Configure Viewport
    D3D11_VIEWPORT vp;
    vp.Width = 800;
    vp.Height = 600;
    vp.MinDepth = 0;
    vp.MaxDepth = 1;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    context->RSSetViewports(1u, &vp);


    // Issue the draw command to draw the verticies
    context->DrawIndexed((UINT)std::size(indices), 0u, 0u);
}


// Graphics exception stuff
Graphics::HrException::HrException( int line,const char * file,HRESULT hr,std::vector<std::string> infoMsgs ) noexcept
	:
	Exception( line,file ),
	hr( hr )
{
	// join all info messages with newlines into single string
	for( const auto& m : infoMsgs )
	{
		info += m;
		info.push_back( '\n' );
	}
	// remove final newline if exists
	if( !info.empty() )
	{
		info.pop_back();
	}
}

const char* Graphics::HrException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "[Error Code] 0x" << std::hex << std::uppercase << GetErrorCode()
		<< std::dec << " (" << (unsigned long)GetErrorCode() << ")" << std::endl
		<< "[Error String] " << GetErrorString() << std::endl
		<< "[Description] " << GetErrorDescription() << std::endl;
	if( !info.empty() )
	{
		oss << "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	}
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::HrException::GetType() const noexcept
{
	return "Chili Graphics Exception";
}

HRESULT Graphics::HrException::GetErrorCode() const noexcept
{
	return hr;
}

std::string Graphics::HrException::GetErrorString() const noexcept
{
	return DXGetErrorString( hr );
}

std::string Graphics::HrException::GetErrorDescription() const noexcept
{
	char buf[512];
	DXGetErrorDescription( hr,buf,sizeof( buf ) );
	return buf;
}

std::string Graphics::HrException::GetErrorInfo() const noexcept
{
	return info;
}


const char* Graphics::DeviceRemovedException::GetType() const noexcept
{
	return "Chili Graphics Exception [Device Removed] (DXGI_ERROR_DEVICE_REMOVED)";
}
Graphics::InfoException::InfoException( int line,const char * file,std::vector<std::string> infoMsgs ) noexcept
	:
	Exception( line,file )
{
	// join all info messages with newlines into single string
	for( const auto& m : infoMsgs )
	{
		info += m;
		info.push_back( '\n' );
	}
	// remove final newline if exists
	if( !info.empty() )
	{
		info.pop_back();
	}
}


const char* Graphics::InfoException::what() const noexcept
{
	std::ostringstream oss;
	oss << GetType() << std::endl
		<< "\n[Error Info]\n" << GetErrorInfo() << std::endl << std::endl;
	oss << GetOriginString();
	whatBuffer = oss.str();
	return whatBuffer.c_str();
}

const char* Graphics::InfoException::GetType() const noexcept
{
	return "Chili Graphics Info Exception";
}

std::string Graphics::InfoException::GetErrorInfo() const noexcept
{
	return info;
}

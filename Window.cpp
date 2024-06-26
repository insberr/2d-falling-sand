//
// Created by insberr on 4/2/2024.
//

#include <Windows.h>
#include "Window.h"
#include <string>
#include <sstream>

#include "ErrorHandling/WindowsThrowMacros.h"
#include "./imgui/imgui.h"
#include "./imgui/imgui_impl_win32.h"
#include "./imgui/imgui_impl_dx11.h"

Window::WindowClass Window::WindowClass::wndClass;

Window::WindowClass::WindowClass() noexcept :
    hInst(GetModuleHandle(nullptr))
{
    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(wc);
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = HandleMsgSetup;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetInstance();
    wc.hIcon = nullptr;
    wc.hCursor = nullptr;
    wc.hbrBackground = nullptr;
    wc.lpszMenuName = nullptr;
    wc.lpszClassName = GetName();
    wc.hIconSm = nullptr;

    // Register the window class
    RegisterClassEx(&wc);
}

Window::WindowClass::~WindowClass() {
    // Unregister the window with windows
    // This makes us a good programmer
    UnregisterClass(wndClassName, GetInstance());
}

const char *Window::WindowClass::GetName() noexcept {
    return wndClassName;
}

HINSTANCE Window::WindowClass::GetInstance() noexcept {
    return wndClass.hInst;
}

Window::Window(int width, int height, const char *name) noexcept :
    width(width), height(height)
{
    RECT wr;
    wr.left = 100;
    wr.right = width + wr.left;
    wr.top = 100;
    wr.bottom = height + wr.top;

    if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0) {
        throw CHWND_LAST_EXCEPT();
    }

    hWnd = CreateWindow(
        WindowClass::GetName(), name,
        WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
        nullptr, nullptr, WindowClass::GetInstance(), this
    );

    if (hWnd == nullptr) {
        throw CHWND_LAST_EXCEPT();
    }

    ShowWindow(hWnd, SW_SHOWDEFAULT);

    // Init ImGui Win32 Impl
     ImGui_ImplWin32_Init(hWnd);

    // Create graphics object
    pGfx = std::make_unique<Graphics>(hWnd);

    // Register the mouse as a raw input device
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = 0;
    rid.hwndTarget = nullptr;
    if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE) {
        throw CHWND_LAST_EXCEPT();
    }
}

Window::~Window() {
     ImGui_ImplWin32_Shutdown();
    // Tell windows to destroy the window
    DestroyWindow(hWnd);
}

LRESULT WINAPI Window::HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    // This message is sent when the window is being created
    if (msg == WM_NCCREATE) {
        const CREATESTRUCTW* const pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        auto const pWnd = static_cast<Window*>(pCreate->lpCreateParams);
        // Set data or something I guess
        // Create a custom piece of data related to our window
        SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWnd));
        SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgThunk));

        // Send that message right on over to our message handler
        return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

LRESULT WINAPI Window::HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
    // Get the window class
    auto const pWnd = reinterpret_cast<Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

    // Snd that message right on over to our code again
    return pWnd->HandleMsg(hWnd, msg, wParam, lParam);
}

LRESULT Window::HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept {
     extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
     if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
     const auto& imio = ImGui::GetIO();

    switch (msg) {
        case WM_CLOSE:
            PostQuitMessage(0);
            return 0;
        // clear keystate when window loses focus to prevent input getting "stuck"
        case WM_KILLFOCUS:
            kbd.ClearState();
            break;
//        case WM_CHAR:
//            static std::string str;
//            str += wParam;
//            SetWindowText(hWnd, str.c_str());
//            break;
            /*********** KEYBOARD MESSAGES ***********/
        case WM_KEYDOWN:
            // syskey commands need to be handled to track ALT key (VK_MENU) and F10
        case WM_SYSKEYDOWN:
            // If ImGui wants control over keyboard input
            if (imio.WantCaptureKeyboard) { break; }

            if( !(lParam & 0x40000000) || kbd.AutorepeatIsEnabled() ) // filter autorepeat
            {
                kbd.OnKeyPressed( static_cast<unsigned char>(wParam) );
            }
            break;
        case WM_KEYUP:
        case WM_SYSKEYUP:
            kbd.OnKeyReleased( static_cast<unsigned char>(wParam) );
            break;
        case WM_CHAR:
            kbd.OnChar( static_cast<unsigned char>(wParam) );
            break;
        /*********** END KEYBOARD MESSAGES ***********/

        /************* MOUSE MESSAGES ****************/
        case WM_MOUSEMOVE:
        {
            if (imio.WantCaptureMouse) { break; }
            const POINTS pt = MAKEPOINTS( lParam );
            // in client region -> log move, and log enter + capture mouse (if not previously in window)
            if( pt.x >= 0 && pt.x < width && pt.y >= 0 && pt.y < height ) {
                mouse.OnMouseMove( pt.x,pt.y );
                if( !mouse.IsInWindow() ) {
                    SetCapture( hWnd );
                    mouse.OnMouseEnter();
                }
            }
            // not in client -> log move / maintain capture if button down
            else
            {
                if( wParam & (MK_LBUTTON | MK_RBUTTON) ) {
                    mouse.OnMouseMove( pt.x,pt.y );
                }
                // button up -> release capture / log event for leaving
                else {
                    ReleaseCapture();
                    mouse.OnMouseLeave();
                }
            }
            break;
        }
        case WM_LBUTTONDOWN:
        {
            SetForegroundWindow( hWnd );

            const POINTS pt = MAKEPOINTS( lParam );
            mouse.OnLeftPressed( pt.x,pt.y );
            break;
        }
        case WM_RBUTTONDOWN:
        {
            const POINTS pt = MAKEPOINTS( lParam );
            mouse.OnRightPressed( pt.x,pt.y );
            break;
        }
        case WM_LBUTTONUP:
        {
            const POINTS pt = MAKEPOINTS( lParam );
            mouse.OnLeftReleased( pt.x,pt.y );
            // release mouse if outside of window
            if( pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height )
            {
                ReleaseCapture();
                mouse.OnMouseLeave();
            }
            break;
        }
        case WM_RBUTTONUP:
        {
            const POINTS pt = MAKEPOINTS( lParam );
            mouse.OnRightReleased( pt.x,pt.y );
            // release mouse if outside of window
            if( pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height )
            {
                ReleaseCapture();
                mouse.OnMouseLeave();
            }
            break;
        }
        case WM_MOUSEWHEEL:
        {
            const POINTS pt = MAKEPOINTS( lParam );
            const int delta = GET_WHEEL_DELTA_WPARAM( wParam );
            mouse.OnWheelDelta( pt.x,pt.y,delta );
            break;
        }
        /************** END MOUSE MESSAGES **************/

        /************** RAW MOUSE MESSAGES **************/
        case WM_INPUT:
        {
            if( !mouse.RawEnabled() )
            {
                break;
            }
            UINT size;
            // first get the size of the input data
            if( GetRawInputData(
                    reinterpret_cast<HRAWINPUT>(lParam),
                    RID_INPUT,
                    nullptr,
                    &size,
                    sizeof( RAWINPUTHEADER ) ) == -1)
            {
                // bail msg processing if error
                break;
            }
            rawBuffer.resize( size );
            // read in the input data
            if( GetRawInputData(
                    reinterpret_cast<HRAWINPUT>(lParam),
                    RID_INPUT,
                    rawBuffer.data(),
                    &size,
                    sizeof( RAWINPUTHEADER ) ) != size )
            {
                // bail msg processing if error
                break;
            }
            // process the raw input data
            auto& ri = reinterpret_cast<const RAWINPUT&>(*rawBuffer.data());
            if( ri.header.dwType == RIM_TYPEMOUSE &&
                (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0) )
            {
                mouse.OnRawDelta( ri.data.mouse.lLastX,ri.data.mouse.lLastY );
            }
            break;
        }
        /************** END RAW MOUSE MESSAGES **************/
    }

    return DefWindowProc(hWnd, msg, wParam, lParam);
}

void Window::SetTitle(const std::string &title) {
    if (SetWindowText(hWnd, title.c_str()) == 0) {
        throw CHWND_LAST_EXCEPT();
    }
}

std::optional<int> Window::ProcessMessages() {
    MSG msg;

    // While queue has messages, remove and dispatch them
    while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
        // Check if quit
        if (msg.message == WM_QUIT) {
            return msg.wParam;
        }

        // Translate
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Return empty optional
    return {};
}

Graphics &Window::Gfx() {
    if (!pGfx) {
        throw CHWND_NOGFX_EXCEPT();
    }
    return *pGfx;
}

void Window::EnableCursor() noexcept {
    cursorEnabled = true;
    ShowCursor();
    EnableImGuiMouse();
    FreeCursor();
}

void Window::DisableCursor() noexcept {
    cursorEnabled = false;
    HideCursor();
    DisableImguiMouse();
    ConfineCursor();
}

bool Window::CursorEnabled() const noexcept {
    return cursorEnabled;
}

void Window::ConfineCursor() noexcept {
    RECT rect;
    GetClientRect(hWnd, &rect);
    MapWindowPoints(hWnd, nullptr, reinterpret_cast<POINT*>(&rect), 2);
    ClipCursor(&rect);
}

void Window::FreeCursor() noexcept {
    ClipCursor(nullptr);
}

void Window::ShowCursor() noexcept {
    while ( ::ShowCursor(TRUE) < 0 );
}

void Window::HideCursor() noexcept {
    while ( ::ShowCursor(FALSE) >= 0);
}

void Window::EnableImGuiMouse() noexcept {
    ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void Window::DisableImguiMouse() noexcept {
    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

// Just don't worry about it.
std::string Window::Exception::TranslateErrorCode(HRESULT hr) noexcept {
    char* pMsgBuf = nullptr;
    DWORD nMsgLen = FormatMessage(
            FORMAT_MESSAGE_ALLOCATE_BUFFER |
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            reinterpret_cast<LPSTR>(&pMsgBuf), 0, nullptr
    );

    if (nMsgLen == 0) {
        return "Unidentified error code from Windows";
    }

    std::string errorString = pMsgBuf;
    LocalFree(pMsgBuf);
    return errorString;
}

Window::HrException::HrException(int line, const char *file, HRESULT hr) noexcept :
        Exception(line, file), hr(hr)
{}

const char *Window::HrException::what() const noexcept {
    std::ostringstream oss;
    oss << GetType() << '\n'
        << "Error [" << GetErrorCode() << " ;"
        << GetOriginString() << "]: "
        << GetErrorString();

    whatBuffer = oss.str();

    return whatBuffer.c_str();
}
const char *Window::HrException::GetType() const noexcept {
    return "Engine Window Exception";
}

HRESULT Window::HrException::GetErrorCode() const noexcept {
    return hr;
}

std::string Window::HrException::GetErrorString() const noexcept {
    return TranslateErrorCode(hr);
}

const char *Window::NoGfxException::GetType() const noexcept {
    return "Engine No GFX Exception";
}

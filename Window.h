//
// Created by insberr on 4/2/2024.
//

#pragma once

#include "ErrorHandling/EngineException.h"
#include <Windows.h>
#include "Input/Keyboard.h"
#include "Input/Mouse.h"
#include "Graphics.h"
#include <optional>
#include <memory>



class Window {
private:
    // Exception For Windowing Stuff
    class Exception : public EngineException {
    public:
        using EngineException::EngineException;
        static std::string TranslateErrorCode(HRESULT hr) noexcept;
    };
public:
    class HrException : public Exception {
    public:
        HrException(int line, const char* file, HRESULT hr) noexcept;
        const char* what() const noexcept override;
        const char* GetType() const noexcept override;

        HRESULT GetErrorCode() const noexcept;
        std::string GetErrorString() const noexcept;
    private:
        HRESULT hr;
    };
    class NoGfxException : public Exception {
    public:
        using Exception::Exception;
        const char* GetType() const noexcept override;
    };
private:

    // Singleton class
    class WindowClass {
    public:
        static const char* GetName() noexcept;
        static HINSTANCE GetInstance() noexcept;

        WindowClass(const WindowClass&) = delete;
        WindowClass& operator=(const WindowClass&) = delete;
    private:
        WindowClass() noexcept;
        ~WindowClass();
        static constexpr const char* wndClassName = "Insberr D3D Engine Window";
        static WindowClass wndClass;
        HINSTANCE hInst;
    };
public:
    Window(int width, int height, const char* name) noexcept;
    ~Window();
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;
    void SetTitle(const std::string& title);
    static std::optional<int> ProcessMessages();
    Graphics& Gfx();
private:
    static LRESULT CALLBACK HandleMsgSetup(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    static LRESULT CALLBACK HandleMsgThunk(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
    LRESULT HandleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) noexcept;
public:
    Keyboard kbd;
    Mouse mouse;
private:
    int width;
    int height;
    HWND hWnd;
    std::unique_ptr<Graphics> pGfx;
};

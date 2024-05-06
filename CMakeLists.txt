cmake_minimum_required(VERSION 3.28)
project(2dfallingsand)

set(CMAKE_CXX_STANDARD 23)

configure_file(Shaders/PixelShader.hlsl ${CMAKE_BINARY_DIR}/PixelShader.hlsl COPYONLY)
configure_file(Shaders/VertexShader.hlsl ${CMAKE_BINARY_DIR}/VertexShader.hlsl COPYONLY)

add_compile_definitions(IS_DEBUG)

add_executable(2dfallingsand
    main.cpp
    App.h
    App.cpp
    Window.cpp
        ErrorHandling/EngineException.cpp
        ErrorHandling/EngineException.h
        ErrorHandling/GraphicsThrowMacros.h
        ErrorHandling/dxerr.cpp
        Graphics.cpp
        Graphics.h
        EngineTimer.cpp
        EngineTimer.h
        Input/Keyboard.cpp
        Input/Mouse.cpp
        DxgiInfoManager.cpp
        DxgiInfoManager.h
        Pixels/Pixels.cpp
        Pixels/Pixels.h
    Camera.cpp
    Camera.h

        #Imgui
        imgui/imgui_impl_dx11.cpp
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_impl_win32.cpp
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imgui.cpp
        imgui/imgui_internal.h
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
        ImguiManager.cpp
        ImguiManager.h

        # Other
)

set_property(TARGET 2dfallingsand PROPERTY WIN32_EXECUTABLE true)

#include "Window.h"
#include <sstream>

#include "App.h"
#include "ErrorHandling/EngineException.h"

int CALLBACK WinMain(
        HINSTANCE hInstance,
        // This one is always null, you can just ignore it's very existence
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nCmdShow
) {
    try {
        return App{}.Run();
    } catch(const EngineException& e) {
        MessageBox(nullptr, e.what(), e.GetType(), MB_OK | MB_ICONEXCLAMATION);
    } catch(const std::exception& e) {
        MessageBox(nullptr, e.what(), "Standard Exception", MB_OK | MB_ICONEXCLAMATION);
    } catch(...) {
        // other unknown exception
        MessageBox(nullptr, "No Error Details Available", "Unknown Exception", MB_OK | MB_ICONEXCLAMATION);
    }

    return -1;
}

#ifdef BUILD_DLL

#include "Graphics.h"
#include "KeyStroke.h"
#include "Utils.h"
#include "Timers.h"
#include "Dll.h"
#include "MainWindow.h"

HWND dllWindow = nullptr;
ATOM registeredWindowClass = 0;

int windowInit() {
    WNDCLASS windowClass = {
        CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        MainWndProc,
        0,
        0,
        dllModule,
        NULL,
        LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL,
        windowClassName
    };
    ATOM rwc = RegisterClass(&windowClass);
    if (rwc == 0) {
        printError("failed to register class!");
        return 0;
    }
    registeredWindowClass = rwc;
    HWND dw = CreateWindow(
        //(LPCTSTR) (DWORD) registeredWindowClass,
        windowClassName,
        windowTitle,
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (HWND) NULL,
        (HMENU) NULL,
        dllModule,
        NULL
    );
    if (dw == NULL) {
        printError("failed to create window!");
        return 0;
    } else {
        dllWindow = dw;
        return 1;
    }
}

VOID CALLBACK RepaintTimer(HWND, UINT, UINT, DWORD) {
    try {
        if (glRepaint()) {
            SwapBuffers(hdc);
        }
    } catch (std::exception & ex) {
        cout << "exception@" << ex.what() << endl;
    }
}

int startMessageLoop() {
    MSG msg;
    if (!SetTimer(dllWindow, MAINWINDOW_REPAINT_TIMER, 1000 / 60, RepaintTimer)) {
        printError("failed to create a screen refresh timer!");
        PostQuitMessage(0);
        return 0;
    }
    int go_on = 1;
    do {
        go_on = GetMessage(&msg, (HWND) NULL, 0, 0);
        if (go_on >= 0) { //no error
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } while (go_on); //not WM_QUIT
    return 1;
}

LRESULT CALLBACK MainWndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) {
    try {
        switch (msg) {
        case WM_CREATE:
            break;
        case WM_CLOSE:
            DestroyWindow(window);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_QUIT:
    //        cleanUp();
            break;
        case WM_SIZE:
            RECT rect;
            if (GetClientRect(window, &rect)) {
                glResize(rect.right - rect.left, rect.bottom - rect. top);
            }
            break;
        case WM_KEYUP:
            keyUp(window, wparam, lparam);
            break;
        case WM_KEYDOWN:
            keyDown(window, wparam, lparam);
            break;
        case WM_SYSKEYUP:
            keyUp(window, wparam, lparam);
            break;
        case WM_SYSKEYDOWN:
            keyDown(window, wparam, lparam);
            break;
        default:
            return DefWindowProc(window, msg, wparam, lparam);
        }
    } catch (std::exception & ex) {
        cout << "exception@" << ex.what() << endl;
    }
    return 0;
}

#endif

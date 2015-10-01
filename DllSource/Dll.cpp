#ifdef BUILD_DLL

//#define FORCE_CONSOLE

#if defined(DEBUG) || defined(FORCE_CONSOLE)
#define WINVER 0x0501   // fix mingw weird errors
#endif
#include "Types.h"
typedef int64 off64_t;

#include <iostream>
#include <sstream>
#include <fcntl.h>
#include <io.h>
#include <stdio.h>
#include <windows.h>
#include "DwarfDataInit.h"
#include "DwarfFortress.h"
#include "Timers.h"
#include "MainWindow.h"
#include "Graphics.h"

#if defined(DEBUG) || defined(FORCE_CONSOLE)
extern "C" _CRTIMP FILE* __cdecl __MINGW_NOTHROW _fdopen (int, const char*); // fix mingw weird errors
#endif

using namespace std;

#define DLLEXTERN extern "C" __declspec(dllexport)

HMODULE dllModule = 0;

void cleanUp() {
    cout << "cleaning up..." << endl;
#if defined(DEBUG) || defined(FORCE_CONSOLE)
    EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND | MF_ENABLED);
    DrawMenuBar(GetConsoleWindow());
    cout << "freeing the console..." << endl;
    if (!FreeConsole()) {
        cout << "failed to free the console, error code " << GetLastError() << endl;
    }
#endif
    if (hud_font) {
        DeleteObject(hud_font);
        hud_font = 0;
    }
    if (text_font) {
        DeleteObject(text_font);
        text_font = 0;
    }
    if (hud_bitmap) {
        DeleteObject(hud_bitmap);
        hud_bitmap = 0;
    }
    if (text_bitmap) {
        DeleteObject(text_bitmap);
        text_bitmap = 0;
    }
    if (hud) {
        DeleteDC(hud);
        hud = 0;
    }
    if (hdc_text) {
        DeleteDC(hdc_text);
        hdc_text = 0;
    }
    if (hrc) {
        wglDeleteContext(hrc);
        hrc = 0;
    }
    if (registeredWindowClass) {
        UnregisterClass((LPCTSTR) (DWORD) registeredWindowClass, dllModule);
    }
    KillTimer(dllWindow, KEYSTROKE_REPEAT_TIMER);
    KillTimer(dllWindow, MAINWINDOW_REPAINT_TIMER);
}

void DllExit(DWORD exitcode) {
//    cleanUp();
    PostQuitMessage(exitcode);
}

string currentDir;

DLLEXTERN DWORD DllEntry(LPVOID v) {
    struct PARAM {
        DWORD processId;
        char * currentDir;
    } * param = (PARAM *) v;
#if defined(DEBUG) || defined(FORCE_CONSOLE)
    DWORD callingProcess = param->processId;
    if (!AttachConsole(callingProcess)) {
        stringstream s;
        s << "cannot attach console, error code: " << GetLastError() << endl;
        MessageBox(0, s.str().data(), "Info", 0);
    } else {
        HANDLE hout = GetStdHandle(STD_OUTPUT_HANDLE);
        int hosf = _open_osfhandle((intptr_t) hout, _O_TEXT);
        FILE * file = _fdopen(hosf, "w");
        *stdout = *file;
        ios::sync_with_stdio(true);
        cout << "console attached" << endl;
    }
    EnableMenuItem(GetSystemMenu(GetConsoleWindow(), FALSE), SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
    DrawMenuBar(GetConsoleWindow());
#endif
    cout << "dll main thread started" << endl;
    currentDir = param->currentDir;
    if (currentDir.back() != '/' && currentDir.back() != '\\') {
        currentDir.push_back('\\');
    }
    cout << "working dir: \"" << currentDir << "\"" << endl;
    if (dwarfDataInit() &&
        windowInit() &&
        glInit(dllWindow)) {
        ShowWindow(dllWindow, SW_SHOWDEFAULT);
        SetFocus(dllWindow);
        UpdateWindow(dllWindow);
        startMessageLoop();
    }
    return 0;
}

BOOL WINAPI DllMain(HINSTANCE dll, DWORD reason, LPVOID) {
    switch (reason) {
    case DLL_PROCESS_ATTACH:
        dllModule = (HMODULE) dll;
        break;
    case DLL_PROCESS_DETACH:
        cleanUp();
        break;
    }
    return TRUE;
}

#endif

#ifdef BUILD_DLL

#ifndef __DORFYVISION__MAINWINDOW_H__
#define __DORFYVISION__MAINWINDOW_H__

#include <windows.h>

char constexpr windowClassName[] = "DorfyVision";
char constexpr windowTitle[] = "Dorfy Vision";

extern HWND dllWindow;
extern ATOM registeredWindowClass;

int windowInit();
int startMessageLoop();
LRESULT CALLBACK MainWndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam);

#endif

#endif

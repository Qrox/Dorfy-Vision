#ifdef BUILD_DLL

#ifndef __DORFY_VISION__KEYSTROKE_H__
#define __DORFY_VISION__KEYSTROKE_H__

#include <windows.h>

void keyDown(HWND window, WPARAM wparam, LPARAM lparam);
void keyUp(HWND window, WPARAM wparam, LPARAM lparam);
VOID CALLBACK keyRepeat(HWND, UINT, UINT, DWORD);

#endif

#endif

#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DLL_H__
#define __DORFY_VISION__DLL_H__

#include <string>
#include <windows.h>

extern HMODULE dllModule;
extern string currentDir;

void cleanUp();
__attribute__((noreturn)) void DllExit(DWORD exitcode);

#endif

#endif

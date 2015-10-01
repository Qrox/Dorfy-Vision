#ifdef BUILD_DLL

#include <iostream>
#include "DwarfDataInit.h"
#include "DwarfTile.h"

using namespace std;

DwarfFortress::Game * pDF;
HMODULE dfModule;
HWND dfWindow;

int32 dwarfDataInit() {
    dfModule = GetModuleHandle("Dwarf Fortress.exe");
    if (!dfModule) {
        return false;
    }
    dfWindow = FindWindow(NULL, "Dwarf Fortress");
    if (!dfWindow) {
        cout << "Failed to find the game window. Key strokes will not be forwarded" << endl;
    }
    pDF = (DwarfFortress::Game *) dfModule;

    DwarfFortress::tileDefinitionInit();
    return true;
}

#endif

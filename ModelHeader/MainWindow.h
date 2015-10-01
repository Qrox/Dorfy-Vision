#ifdef BUILD_MODEL

#ifndef __DORFY_VISION__MODEL__MAIN_WINDOW_H__
#define __DORFY_VISION__MODEL__MAIN_WINDOW_H__

#include <iostream>
#include <sstream>
#include <memory>
#include <vector>
#include <set>
#include <windows.h>
#include <gl/gl.h>
#include "Vector3f.h"
#include "Primitive.h"
#include "Types.h"

using namespace std;


extern HWND modelWindow;
extern BOOL lock_mouse;

extern dword cmdcursor;
extern string cmdline;

class primitiveselection {
public:
    float scrz;
    primitive * prim;

    primitiveselection(float scrz, primitive * prim) : scrz(scrz), prim(prim) {}
};

class primitivecomp {
public:
    bool operator()(primitive * a, primitive * b) {
        return (ptrword) a < (ptrword) b;
    }
};

extern set<primitive *, primitivecomp> primitives;
extern vector<primitiveselection> selectedprimitives;
extern dword selectionindex;

int windowInit();
int startMessageLoop();

#endif

#endif

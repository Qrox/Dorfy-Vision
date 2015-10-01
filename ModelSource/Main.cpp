#ifdef BUILD_MODEL

#include <iostream>
#include <cmath>
#include "MainWindow.h"
#include "Graphics.h"

using namespace std;

int main(void) {
    windowInit();
    glInit(modelWindow);
    ShowWindow(modelWindow, SW_SHOWDEFAULT);
    SetFocus(modelWindow);
    UpdateWindow(modelWindow);
//    FreeConsole();
    // corner vertices & normal of 6 faces
    primitives.insert({
        new point(-.5, -.5, 0),
        new point(-.5, -.5, 1),
        new point(-.5,  .5, 0),
        new point(-.5,  .5, 1),
        new point( .5, -.5, 0),
        new point( .5, -.5, 1),
        new point( .5,  .5, 0),
        new point( .5,  .5, 1),
        new line(vector3f(0, 0, .5), vector3f( .1,   0, .5)),
        new line(vector3f(0, 0, .5), vector3f(-.1,   0, .5)),
        new line(vector3f(0, 0, .5), vector3f(  0,  .1, .5)),
        new line(vector3f(0, 0, .5), vector3f(  0, -.1, .5)),
        new line(vector3f(0, 0, .5), vector3f(  0,   0, .6)),
        new line(vector3f(0, 0, .5), vector3f(  0,   0, .4)),
    });

//    for (dword i = 0; i < 10; ++i) {
//        float x = (float) rand() / RAND_MAX - .5;
//        float y = (float) rand() / RAND_MAX - .5;
//        float face_tan_x = (float) rand() / RAND_MAX * 2 - 1;
//        float face_tan_y = (float) rand() / RAND_MAX * 2 - 1;
//        float width = (float) rand() / RAND_MAX * .01 + .095;
//        float k = (float) rand() / RAND_MAX * .01 - .005;
//        vector<vector3f> cmd;
//        for (dword h = 0; h < 4; ++h) {
//            float w = (1 - exp(h - 4)) / (1 - exp(-4));
//
//        }
//    }
    startMessageLoop();
    for (auto i = primitives.begin(); i != primitives.end(); ++i) {
        delete *i;
    }
    return 0;
}

#endif

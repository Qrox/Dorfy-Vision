#ifdef BUILD_DLL

#ifndef __DORFY_VISION__UTILS_H__
#define __DORFY_VISION__UTILS_H__

#include <iostream>
#include <iomanip>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

using namespace std;

#undef M_PI
float constexpr M_PI = 3.14159265358979323846;

class vector2d {
public:
    double x, y;

    vector2d(double x, double y);
    double cross(vector2d v);
    double mod();
    vector2d operator +(vector2d v);
    vector2d operator -(vector2d v);
    vector2d operator *(double v);
    vector2d rotate(double angle);
    void operator +=(vector2d v);
    void operator -=(vector2d v);
};

vector2d lineIntersectionPoint(vector2d a0, vector2d a1, vector2d b0, vector2d b1);

template <typename Tp>
Tp limit(Tp min, Tp v, Tp max) {
    return v < min ? min : v > max ? max : v;
}

template <typename Tp>
Tp sqr(Tp const & v) {
    return v * v;
}

void solveBinaryEquation(float v[2][4]);
void printError(string msg);

#endif

#endif

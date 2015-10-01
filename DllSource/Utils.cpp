#if BUILD_DLL

#include <iostream>
#include <windows.h>
#include <cmath>
#include "Utils.h"

using namespace std;

vector2d::vector2d(double x, double y) : x(x), y(y) {}

double vector2d::cross(vector2d v) {
    return x * v.y - y * v.x;
}

double vector2d::mod() {
    return sqrt(x * x + y * y);
}

vector2d vector2d::rotate(double angle) {
    return *this * cos(angle) + vector2d(-y, x) * sin(angle);
}

vector2d vector2d::operator +(vector2d v) {
    return {x + v.x, y + v.y};
}

vector2d vector2d::operator -(vector2d v) {
    return {x - v.x, y - v.y};
}

vector2d vector2d::operator *(double v) {
    return {x * v, y * v};
}

void vector2d::operator +=(vector2d v) {
    x += v.x;
    y += v.y;
}

void vector2d::operator -=(vector2d v) {
    x -= v.x;
    y -= v.y;
}

vector2d lineIntersectionPoint(vector2d a0, vector2d a1, vector2d b0, vector2d b1) {
    vector2d a = a1 - a0, b = b1 - b0;
    double den = a.cross(b);
    double a0_a = a0.cross(a), b0_b = b0.cross(b);
    return {(a.x * b0_b - b.x * a0_a) / den, (a.y * b0_b - b.y * a0_a) / den};
}

void solveBinaryEquation(float v[2][4]) {
    v[0][3] = (v[0][1] * v[1][2] - v[1][1] * v[0][2]) / (v[1][1] * v[0][0] - v[0][1] * v[1][0]);
    v[1][3] = (v[0][0] * v[1][2] - v[1][0] * v[0][2]) / (v[1][0] * v[0][1] - v[0][0] * v[1][1]);
}

void printError(string msg) {
    cout << msg << endl;
    DWORD error = GetLastError();
    if (error != 0) {
        cout << "(error code " << error << ")" << endl;
    }
}

#endif

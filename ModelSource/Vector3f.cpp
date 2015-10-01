#ifdef BUILD_MODEL

#include <iostream>
#include <sstream>
#include <cmath>
#include "Vector3f.h"
#include "Types.h"

using namespace std;

vector3f const vector3f::ex = vector3f(1, 0, 0);
vector3f const vector3f::ey = vector3f(0, 1, 0);
vector3f const vector3f::ez = vector3f(0, 0, 1);
vector3f const vector3f::zero = vector3f(0, 0, 0);

vector3f::vector3f() {
    v[0] = 0;
    v[1] = 0;
    v[2] = 0;
}

vector3f::vector3f(string s) {
    istringstream str(s);
    string::size_type begin = 0;
    dword i = 0;
    for (; i < 3; ++i) {
        begin = s.find_first_of("0123456789+-.", begin);
        if (begin == string::npos) break;
        str.seekg(begin);
        str >> v[i];
        begin = str.tellg();
    }
    for (; i < 3; ++i) {
        v[i] = 0;
    }
}

vector3f::vector3f(initializer_list<float> d) {
    auto size = d.size();
    if (size > 0) v[0] = * d.begin();      else v[0] = 0;
    if (size > 1) v[1] = *(d.begin() + 1); else v[1] = 0;
    if (size > 2) v[2] = *(d.begin() + 2); else v[2] = 0;
}

vector3f::vector3f(float x, float y, float z) {
    v[0] = x;
    v[1] = y;
    v[2] = z;
}

vector3f::vector3f(float const * in) {
    v[0] = in[0];
    v[1] = in[1];
    v[2] = in[2];
}

vector3f::vector3f(double const * in) {
    v[0] = in[0];
    v[1] = in[1];
    v[2] = in[2];
}

vector3f::vector3f(vector3f const & w) noexcept {
    v[0] = w[0];
    v[1] = w[1];
    v[2] = w[2];
}

vector3f & vector3f::operator =(vector3f const & w) {
    v[0] = w[0];
    v[1] = w[1];
    v[2] = w[2];
    return *this;
}

vector3f::operator string() const {
    ostringstream str;
    str << "(" << v[0] << ", " << v[1] << ", " << v[2] << ")";
    return str.str();
}

vector3f::operator float const *() const {
    return v;
}

float const vector3f::operator [](int i) const {
    return v[i];
}

float vector3f::mod() const {
    return sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]);
}

float vector3f::dot(vector3f const & w) const {
    return v[0] * w[0] +
           v[1] * w[1] +
           v[2] * w[2];
}

vector3f vector3f::normalize() const {
    float len = mod();
    if (len == 0) return zero;
    else return *this / len;
}

bool vector3f::operator ==(vector3f const & w) const {
    return v[0] == w[0] && v[1] == w[1] && v[2] == w[2];
}

bool vector3f::operator !=(vector3f const & w) const {
    return v[0] != w[0] || v[1] != w[1] || v[2] != w[2];
}

vector3f vector3f::operator -() const {
    return vector3f(-v[0], -v[1], -v[2]);
}

vector3f vector3f::operator -(vector3f const & w) const {
    return vector3f(v[0] - w[0], v[1] - w[1], v[2] - w[2]);
}

vector3f vector3f::operator +(vector3f const & w) const {
    return vector3f(v[0] + w[0], v[1] + w[1], v[2] + w[2]);
}

vector3f vector3f::operator *(float d) const {
    return vector3f(v[0] * d, v[1] * d, v[2] * d);
}

vector3f vector3f::operator /(float d) const {
    return vector3f(v[0] / d, v[1] / d, v[2] / d);
}

vector3f vector3f::cross(vector3f const & w) const {
    return vector3f(v[1] * w[2] - v[2] * w[1], v[2] * w[0] - v[0] * w[2], v[0] * w[1] - v[1] * w[0]);
}

vector3f vector3f::angleto(vector3f const & w) const {
    float vmod = mod(), wmod = w.mod();
    if (vmod == 0 || wmod == 0) return zero;
    float angle = acos(dot(w) / vmod / wmod);
    if (angle == 0) return zero;
    return cross(w).normalize() * angle;
}

vector3f vector3f::rotate(vector3f const & rot) const {
//    vector3f const & v = *this;                     // v (this)
//    float mrot = rot.mod();                         // angle of rotation
//    if (mrot == 0) return *this;                    // no rotation (avoid divide by 0)
//    vector3f erot = rot / mrot;                     // axis of rotation
//    vector3f vph = erot.cross(v);                   // 90 degrees rotation of vp
//    vector3f vp = vph.cross(erot);                  // projection of v on the rotation plane
//    vector3f va = v - vp;                           // projection of v on the rotation axis
//    return va + vp * cos(mrot) + vph * sin(mrot);   // va not rotated, vp rotated by mrot, then added together

    // optimized
    float mrot = rot.mod();
    if (mrot == 0) return *this;
    vector3f erot = rot / mrot;
    vector3f vph = erot.cross(*this);
    return (*this + vph * sin(mrot) + vph.cross(erot) * (cos(mrot) - 1)); //.normalize() * mod() if cannot keep a stable length
}

float vector3f::distancetoline(vector3f const & a, vector3f const & b) const {
    return (*this - a).cross((b - a).normalize()).mod();
}

float vector3f::distancetosurface(vector3f const & o, vector3f const & v0, vector3f const & v1) const {
    vector3f normal = (v0 - o).cross(v1 - o).normalize();
    return (*this - o).dot(normal);
}

#endif

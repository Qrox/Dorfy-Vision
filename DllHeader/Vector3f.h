#ifdef BUILD_DLL

#ifndef __DORFY_VISION__VECTOR3F_H__
#define __DORFY_VISION__VECTOR3F_H__

#include <iostream>

using namespace std;

class vector3f {
    float v[3];

public:
    static vector3f const ex, ey, ez, zero;

    vector3f();
    vector3f(float x, float y, float z);
    vector3f(float const * in);
    vector3f(vector3f const & w);

    vector3f & operator =(vector3f const & w);

    operator float const *() const;
    float const operator [](int i) const;

    float mod() const;
    float dot(vector3f const & w) const;
    vector3f normalize() const;
    vector3f operator -() const;
    vector3f operator -(vector3f const & w) const;
    vector3f operator +(vector3f const & w) const;
    vector3f operator *(float d) const;
    vector3f operator /(float d) const;
    vector3f cross(vector3f const & w) const;
    vector3f angleto(vector3f const & w) const;
    vector3f rotate(vector3f const & rot) const;
};

ostream & operator << (ostream & out, vector3f const & v);

#endif

#endif

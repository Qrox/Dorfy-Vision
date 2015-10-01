#ifdef BUILD_MODEL

#ifndef __DORFY_VISION__MODEL__VECTOR3F_H__
#define __DORFY_VISION__MODEL__VECTOR3F_H__

#include <string>
#include <initializer_list>

using namespace std;

class vector3f {
    float v[3];

public:
    static vector3f const ex, ey, ez, zero;

    vector3f();
    vector3f(string s);
    vector3f(initializer_list<float> l);
    vector3f(float x, float y, float z);
    vector3f(float const * in);
    vector3f(double const * in);
    vector3f(vector3f const & w) noexcept;

    vector3f & operator =(vector3f const & w);

    operator string() const;
    operator float const *() const;
    float const operator [](int i) const;

    float mod() const;
    float dot(vector3f const & w) const;
    vector3f normalize() const;
    bool operator ==(vector3f const & w) const;
    bool operator !=(vector3f const & w) const;
    vector3f operator -() const;
    vector3f operator -(vector3f const & w) const;
    vector3f operator +(vector3f const & w) const;
    vector3f operator *(float d) const;
    vector3f operator /(float d) const;
    vector3f cross(vector3f const & w) const;
    vector3f angleto(vector3f const & w) const;
    vector3f rotate(vector3f const & rot) const;
    float distancetoline(vector3f const & a, vector3f const & b) const;
    float distancetosurface(vector3f const & o, vector3f const & v0, vector3f const & v1) const;
};

#endif

#endif

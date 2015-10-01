#ifdef BUILD_MODEL

#ifndef __DORFY_VISION__MODEL__PRIMITIVE_H__
#define __DORFY_VISION__MODEL__PRIMITIVE_H__

#include <windows.h>
#include <gl/gl.h>
#include <gl/glu.h>
#include <string>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <vector>
#include "Vector3f.h"
#include "Graphics.h"
#include "Types.h"

using namespace std;

float constexpr deviation_allowance = 10;
float constexpr sqr_deviation_allowance = deviation_allowance * deviation_allowance;

class primitive {
public:
    typedef vector<string>::iterator::difference_type size_type;

    enum primitivetype {
        points = GL_POINTS,
        lines = GL_LINES,
        line_loop = GL_LINE_LOOP,
        line_strip = GL_LINE_STRIP,
        triangles = GL_TRIANGLES,
        triangle_strip = GL_TRIANGLE_STRIP,
        triangle_fan = GL_TRIANGLE_FAN,
        quads = GL_QUADS,
        quad_strip = GL_QUAD_STRIP,
        polygon = GL_POLYGON,
    };

    virtual ~primitive() {};
    virtual primitivetype type() const = 0;
    virtual vector3f getsizefromorigin() const = 0;
    /* if the primitive is selected by cursor at (x, y),
     * returns true and set z to the distance from the cursor
     * to the nearest point pointed by the cursor */
    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const = 0;
    virtual void glrender() const = 0;
    virtual void glrenderselection() const = 0;

    /* write type and terminate command mode flag (cnt = 0x0000) */
    static void glbegin(ofstream & out, word type) {
        word constexpr cnt = 0;
        out.write((char *) &type, sizeof(type));
        out.write((char *) &cnt, sizeof(cnt));
    }

    /* write type and terminate command mode flag (cnt = 0x0000) */
    virtual void glbegin(ofstream & out) const {
        glbegin(out, type());
    }

    /* write vertices and normals, but not type and cnt */
    virtual void glrender(ofstream & out) const = 0;

    /* write terminate command (0xFFFFFFFF) */
    static void glend(ofstream & out) {
        dword glend = ((dword) 0) - 1;
        out.write((char *) &glend, sizeof(glend));
    }

    static void vertex3fv(ofstream & out, float const * v) {
        dword glvertex3f = 0;
        out.write((char *) &glvertex3f, sizeof(glvertex3f));
        out.write((char *) v, sizeof(float) * 3);
    }

    static void normal3fv(ofstream & out, float const * v) {
        dword glnormal3f = 1;
        out.write((char *) &glnormal3f, sizeof(glnormal3f));
        out.write((char *) v, sizeof(float) * 3);
    }

    static void calcvertexandnormal(string s, vector3f & v, vector3f & n, vector3f * prev_n) {
        auto colon = s.find(':');
        if (colon == string::npos) {
            v = s;
            if (prev_n) n = *prev_n;
            else n = vector3f::zero;
        } else {
            v = s.substr(0, colon);
            n = vector3f(s.substr(colon + 1)).normalize();
        }
    }

    static void glrender(ofstream & out, vector3f const * v, vector3f const * n, size_type cnt) {
        for (size_type i = 0; i < cnt; ++i) {
            if (i == 0 || n[i] != n[i - 1]) {
                normal3fv(out, n[i]);
            }
            vertex3fv(out, v[i]);
        }
    }
};

class point : public primitive {
public:
    vector3f v;
    vector3f n;

    point(float a, float b, float c) : v(a, b, c), n() {}

    point(vector3f v) : v(v), n() {}

    point(string s) {
        calcvertexandnormal(s, v, n, nullptr);
    }

    virtual ~point() {
        cout << "deleting point@" << this << endl;
    }

    virtual primitivetype type() const {
        return points;
    }

    virtual vector3f getsizefromorigin() const {
        return vector3f(abs(v[0]) * 2, abs(v[1]) * 2, max(v[2], 0.f));
    }

    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const {
        GLdouble scr[3];
        gluProject(v[0], v[1], v[2], model, proj, view, &scr[0], &scr[1], &scr[2]);
        scr[0] -= x;
        scr[1] -= y;
        if (scr[0] * scr[0] + scr[1] * scr[1] <= sqr_deviation_allowance) {
            z = scr[2];
            return true;
        } else {
            return false;
        }
    }

    virtual void glrender() const {
        glPointSize(3);
        glBegin(GL_POINTS);
        glNormal3fv(n);
        glVertex3fv(v);
        glEnd();
    }

    virtual void glrenderselection() const {
        glPointSize(5);
        glBegin(GL_POINTS);
        glVertex3fv(v);
        glEnd();
    }

    virtual void glrender(ofstream & out) const {
        primitive::glrender(out, &v, &n, 1);
    }
};

class line : public primitive {
public:
    vector3f v[2];
    vector3f n[2];

    line(vector3f const & v0, vector3f const & v1) : v{v0, v1}, n{vector3f::zero, vector3f::zero} {}

    line(string s0, string s1) {
        calcvertexandnormal(s0, v[0], n[0], nullptr);
        calcvertexandnormal(s1, v[1], n[1], &n[0]);
    }

    virtual ~line() {
        cout << "deleting line@" << this << endl;
    }

    virtual primitivetype type() const {
        return lines;
    }

    virtual vector3f getsizefromorigin() const {
        return vector3f(max(abs(v[0][0]), abs(v[1][0])) * 2, max(abs(v[0][1]), abs(v[1][1])) * 2, max({v[0][2], v[1][2], 0.f}));
    }

    static bool isselected(vector3f const & cursor, vector3f const & _vscra, vector3f const & _vscrb, float & z) {
        vector3f vscra(_vscra[0], _vscra[1], 0), vscrb(_vscrb[0], _vscrb[1], 0);
        float cursordist, cursorpos;
        if (vscra != vscrb) {
            cursordist = cursor.distancetoline(vscra, vscrb); // cursor's projection distance to the line on screen
            cursorpos = (cursor - vscra).dot((vscrb - vscra).normalize()); // cursor's projection position on the line on screen, from a
        } else {
            cursordist = (cursor - vscra).mod();
            cursorpos = 0;
        }
        float scrdist = (vscrb - vscra).mod(); // line's length on screen
        if (cursorpos < 0) { // cursor's projection is away from a
            if (cursorpos * cursorpos + cursordist * cursordist <= sqr_deviation_allowance) {
                z = vscra[2];
                return true;
            }
        } else if (cursorpos >= scrdist) { // cursor's projection is away from b
            float cp = cursorpos - scrdist;
            if (cp * cp + cursordist * cursordist <= sqr_deviation_allowance) {
                z = vscrb[2];
                return true;
            }
        } else if (cursordist <= deviation_allowance) { // cursor's projection is on the line
            z = vscra[2] + (vscrb[2] - vscra[2]) / scrdist * cursorpos;
            return true;
        }
        return false;
    }

    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const {
        double scra[3], scrb[3];
        gluProject(v[0][0], v[0][1], v[0][2], model, proj, view, &scra[0], &scra[1], &scra[2]);
        gluProject(v[1][0], v[1][1], v[1][2], model, proj, view, &scrb[0], &scrb[1], &scrb[2]);
        vector3f cursor(x, y, 0), vscra(scra[0], scra[1], 0), vscrb(scrb[0], scrb[1], 0);
        return isselected(cursor, vscra, vscrb, z);
    }

    virtual void glrender() const {
        glLineWidth(1);
        glBegin(GL_LINES);
        glNormal3fv(n[0]);
        glVertex3fv(v[0]);
        glNormal3fv(n[1]);
        glVertex3fv(v[1]);
        glEnd();
    }

    virtual void glrenderselection() const {
        glLineWidth(3);
        glBegin(GL_LINES);
        glVertex3fv(v[0]);
        glVertex3fv(v[1]);
        glEnd();
        glPointSize(3);
        glBegin(GL_POINTS);
        glVertex3fv(v[0]);
        glVertex3fv(v[1]);
        glEnd();
    }

    virtual void glrender(ofstream & out) const {
        primitive::glrender(out, v, n, 2);
    }
};

class triangle : public primitive {
public:
    vector3f v[3];
    vector3f n[3];

    triangle(string a, string b, string c) {
        calcvertexandnormal(a, v[0], n[0], nullptr);
        calcvertexandnormal(b, v[1], n[1], &n[0]);
        calcvertexandnormal(c, v[2], n[2], &n[1]);
    }

    triangle(vector3f v0, vector3f v1, vector3f v2, vector3f n0, vector3f n1, vector3f n2) : v{v0, v1, v2}, n{n0, n1, n2} {
    }

    virtual ~triangle() {
        cout << "deleting triangle@" << this << endl;
    }

    virtual primitivetype type() const {
        return triangles;
    }

    virtual vector3f getsizefromorigin() const {
        float ret[3] = {0, 0, 0};
        for (dword i = 0; i < 3; ++i) {
            ret[0] = max(abs(v[i][0]), ret[0]);
            ret[1] = max(abs(v[i][1]), ret[1]);
            ret[2] = max(v[i][2], ret[2]);
        }
        ret[0] *= 2; ret[1] *= 2;
        return ret;
    }

    static bool isselected(vector3f const & cursor, vector3f const & vscr0, vector3f const & vscr1, vector3f const & vscr2, float & z) {
        vector3f vscrxy[3] = {
            vector3f(vscr0[0], vscr0[1], 0),
            vector3f(vscr1[0], vscr1[1], 0),
            vector3f(vscr2[0], vscr2[1], 0),
        };
        bool front = (vscrxy[1] - vscrxy[0]).cross(vscrxy[2] - vscrxy[1])[2] >= 0;
        bool render = front || !frustum;
        if (render) { // ccw or design view
            float z0 = (vscrxy[1] - vscrxy[0]).cross(cursor - vscrxy[0])[2],
                  z1 = (vscrxy[2] - vscrxy[1]).cross(cursor - vscrxy[1])[2],
                  z2 = (vscrxy[0] - vscrxy[2]).cross(cursor - vscrxy[2])[2];
            if (front ? (z0 >= 0 && z1 >= 0 && z2 >= 0)
                      : (z0 <= 0 && z1 <= 0 && z2 <= 0)) { // inside
                vector3f vscr3 = vscr0 + (vscr1 - vscr0) * ((cursor[0] - vscr0[0]) / (vscr1[0] - vscr0[0]));
                vector3f vscr4 = vscr0 + (vscr2 - vscr0) * ((cursor[0] - vscr0[0]) / (vscr2[0] - vscr0[0]));
                vector3f vscr5 = vscr3 + (vscr4 - vscr3) * ((cursor[1] - vscr3[1]) / (vscr4[1] - vscr3[1]));
                z = vscr5[2];
                return true;
            } else {
                bool extend = false;
                for (dword i = 0; i < 3; ++i) {
                    dword j = i == 2 ? 0 : i + 1;
                    dword k = i == 0 ? 2 : i - 1;
                    if (vscrxy[i].distancetoline(vscrxy[j], vscrxy[k]) <= deviation_allowance) { // too small, need to extend the hitting range
                        extend = true;
                        break;
                    }
                }
                if (extend) {
                    for (dword i = 0; i < 3; ++i) {
                        dword j = i == 2 ? 0 : i + 1;
                        if (line::isselected(cursor, vscrxy[i], vscrxy[j], z)) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const {
        vector3f cursor(x, y, 0), vscr[3];
        for (dword i = 0; i < 3; ++i) {
            double scr[3];
            gluProject(v[i][0], v[i][1], v[i][2], model, proj, view, &scr[0], &scr[1], &scr[2]);
            vscr[i] = vector3f(scr);
        }
        return isselected(cursor, vscr[0], vscr[1], vscr[2], z);
    }

    virtual void glrender() const {
        glBegin(GL_TRIANGLES);
        for (dword i = 0; i < 3; ++i) {
            glNormal3fv(n[i]);
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrenderselection() const {
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        for (dword i = 0; i < 3; ++i) {
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrender(ofstream & out) const {
        primitive::glrender(out, v, n, 3);
    }
};

class quad : public primitive {
public:
    vector3f v[4];
    vector3f n[4];

    quad(string s0, string s1, string s2, string s3) {
        calcvertexandnormal(s0, v[0], n[0], nullptr);
        calcvertexandnormal(s1, v[1], n[1], &n[0]);
        calcvertexandnormal(s2, v[2], n[2], &n[1]);
        calcvertexandnormal(s3, v[3], n[3], &n[2]);
    }

    quad(vector3f v0, vector3f v1, vector3f v2, vector3f v3, vector3f n0, vector3f n1, vector3f n2, vector3f n3) : v{v0, v1, v2, v3}, n{n0, n1, n2, n3} {
    }

    virtual ~quad() {
        cout << "deleting quad@" << this << endl;
    }

    virtual primitivetype type() const {
        return quads;
    }

    virtual vector3f getsizefromorigin() const {
        float ret[3] = {0, 0, 0};
        for (dword i = 0; i < 4; ++i) {
            ret[0] = max(abs(v[i][0]), ret[0]);
            ret[1] = max(abs(v[i][1]), ret[1]);
            ret[2] = max(v[i][2], ret[2]);
        }
        ret[0] *= 2; ret[1] *= 2;
        return ret;
    }

    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const {
        vector3f cursor(x, y, 0), vscr[4];
        for (dword i = 0; i < 4; ++i) {
            double scr[3];
            gluProject(v[i][0], v[i][1], v[i][2], model, proj, view, &scr[0], &scr[1], &scr[2]);
            vscr[i] = vector3f(scr);
        }
        // todo
        return triangle::isselected(cursor, vscr[3], vscr[0], vscr[1], z) ||
               triangle::isselected(cursor, vscr[1], vscr[2], vscr[3], z);
    }

    virtual void glrender() const {
        glBegin(GL_QUADS);
        for (dword i = 0; i < 4; ++i) {
            glNormal3fv(n[i]);
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrenderselection() const {
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        for (dword i = 0; i < 4; ++i) {
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrender(ofstream & out) const {
        primitive::glrender(out, v, n, 4);
    }
};

class line_loop : public primitive {
public:
    typedef vector<string>::iterator::difference_type size_type;
    size_type cnt;
    vector3f * v;
    vector3f * n;

    line_loop(vector<string>::iterator i, vector<string>::iterator end) {
        cnt = end - i;
        v = new vector3f[cnt];
        n = new vector3f[cnt];
        for (dword j = 0; i < end; ++i, ++j) {
            calcvertexandnormal(*i, v[j], n[j], j == 0 ? nullptr : &n[j - 1]);
        }
    }

    virtual ~line_loop() {
        cout << "deleting line_loop@" << this << endl;
        delete [] v;
    }

    virtual primitivetype type() const {
        return primitive::line_loop;
    }

    virtual vector3f getsizefromorigin() const {
        float ret[3] = {0, 0, 0};
        for (size_type i = 0; i < cnt; ++i) {
            ret[0] = max(abs(v[i][0]), ret[0]);
            ret[1] = max(abs(v[i][1]), ret[1]);
            ret[2] = max(v[i][2], ret[2]);
        }
        ret[0] *= 2; ret[1] *= 2;
        return ret;
    }

    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const {
        vector3f cursor(x, y, 0);
        double scr[2][3];
        double * scr_0 = scr[0], * scr_1 = scr[1];
        gluProject(v[0][0], v[0][1], v[0][2], model, proj, view, &scr[0][0], &scr[0][1], &scr[0][2]);
        for (size_type i = 0; i < cnt; ++i) {
            size_type j = i + 1;
            if (j == cnt) j = 0;
            gluProject(v[j][0], v[j][1], v[j][2], model, proj, view, scr_1, scr_1 + 1, scr_1 + 2);
            if (line::isselected(cursor, vector3f(scr_0), vector3f(scr_1), z)) {
                return true;
            }
            double * tmp = scr_0;
            scr_0 = scr_1;
            scr_1 = tmp;
        }
        return false;
    }

    virtual void glrender() const {
        glLineWidth(1);
        glBegin(GL_LINE_LOOP);
        for (size_type i = 0; i < cnt; ++i) {
            glNormal3fv(n[i]);
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrenderselection() const {
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        for (size_type i = 0; i < cnt; ++i) {
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrender(ofstream & out) const {
        primitive::glrender(out, v, n, cnt);
    }
};

class line_strip : public primitive {
public:
    typedef vector<string>::iterator::difference_type size_type;
    size_type cnt;
    vector3f * v;
    vector3f * n;

    line_strip(vector<string>::iterator i, vector<string>::iterator end) {
        cnt = end - i;
        v = new vector3f[cnt];
        n = new vector3f[cnt];
        for (size_type j = 0; j < cnt; ++i, ++j) {
            calcvertexandnormal(*i, v[j], n[j], j == 0 ? nullptr : &n[j - 1]);
        }
    }

    virtual ~line_strip() {
        cout << "deleting line_strip@" << this << endl;
        delete [] v;
    }

    virtual primitivetype type() const {
        return primitive::line_strip;
    }

    virtual vector3f getsizefromorigin() const {
        float ret[3] = {0, 0, 0};
        for (size_type i = 0; i < cnt; ++i) {
            ret[0] = max(abs(v[i][0]), ret[0]);
            ret[1] = max(abs(v[i][1]), ret[1]);
            ret[2] = max(v[i][2], ret[2]);
        }
        ret[0] *= 2; ret[1] *= 2;
        return ret;
    }

    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const {
        vector3f cursor(x, y, 0);
        double scr[2][3];
        double * scr_0 = scr[0], * scr_1 = scr[1];
        gluProject(v[0][0], v[0][1], v[0][2], model, proj, view, &scr[0][0], &scr[0][1], &scr[0][2]);
        for (size_type j = 1; j < cnt; ++j) {
            gluProject(v[j][0], v[j][1], v[j][2], model, proj, view, scr_1, scr_1 + 1, scr_1 + 2);
            if (line::isselected(cursor, vector3f(scr_0), vector3f(scr_1), z)) {
                return true;
            }
            double * tmp = scr_0;
            scr_0 = scr_1;
            scr_1 = tmp;
        }
        return false;
    }

    virtual void glrender() const {
        glLineWidth(1);
        glBegin(GL_LINE_STRIP);
        for (size_type i = 0; i < cnt; ++i) {
            glVertex3fv(n[i]);
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrenderselection() const {
        glLineWidth(3);
        glBegin(GL_LINE_STRIP);
        for (size_type i = 0; i < cnt; ++i) {
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrender(ofstream & out) const {
        primitive::glrender(out, v, n, cnt);
    }
};

class triangle_strip : public primitive {
public:
    typedef vector<string>::iterator::difference_type size_type;
    size_type cnt;
    vector3f * v;
    vector3f * n;

    triangle_strip(vector<string>::iterator i, vector<string>::iterator end) {
        cnt = end - i;
        v = new vector3f[cnt];
        n = new vector3f[cnt];
        for (size_type j = 0; j < cnt; ++i, ++j) {
            calcvertexandnormal(*i, v[j], n[j], j == 0 ? nullptr : &n[j - 1]);
        }
    }

    triangle_strip(vector<vector3f> const & vs, vector<vector3f> const & ns) {
        cnt = min(vs.size(), ns.size());
        v = new vector3f[cnt];
        n = new vector3f[cnt];
        copy(vs.begin(), vs.begin() + cnt, v);
        copy(ns.begin(), ns.begin() + cnt, n);
    }

    virtual ~triangle_strip() {
        cout << "deleting triangle_strip@" << this << endl;
        delete [] v;
    }

    virtual primitivetype type() const {
        return primitive::triangle_strip;
    }

    virtual vector3f getsizefromorigin() const {
        float ret[3] = {0, 0, 0};
        for (size_type i = 0; i < cnt; ++i) {
            ret[0] = max(abs(v[i][0]), ret[0]);
            ret[1] = max(abs(v[i][1]), ret[1]);
            ret[2] = max(v[i][2], ret[2]);
        }
        ret[0] *= 2; ret[1] *= 2;
        return ret;
    }

    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const {
        vector3f cursor(x, y, 0);
        double scr[3][3];
        gluProject(v[0][0], v[0][1], v[0][2], model, proj, view, &scr[0][0], &scr[0][1], &scr[0][2]);
        gluProject(v[1][0], v[1][1], v[1][2], model, proj, view, &scr[1][0], &scr[1][1], &scr[1][2]);
        double * scr_0 = scr[0], * scr_1 = scr[1], * scr_2 = scr[2];
        for (size_type i = 2; i < cnt; ++i) {
            gluProject(v[i][0], v[i][1], v[i][2], model, proj, view, scr_2, scr_2 + 1, scr_2 + 2);
            if (triangle::isselected(cursor, vector3f(scr_0), vector3f(scr_1), vector3f(scr_2), z)) {
                return true;
            }
            double * tmp;
            if (i & 1) { // swap 1, 2
                tmp = scr_1;
                scr_1 = scr_2;
                scr_2 = tmp;
            } else { // swap 0, 2
                tmp = scr_0;
                scr_0 = scr_2;
                scr_2 = tmp;
            }
        }
        return false;
    }

    virtual void glrender() const {
       glBegin(GL_TRIANGLE_STRIP);
       for (size_type i = 0; i < cnt; ++i) {
           glNormal3fv(n[i]);
           glVertex3fv(v[i]);
       }
       glEnd();
    }

    virtual void glrenderselection() const {
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        for (size_type i = 1; i < cnt; i += 2) {
            glVertex3fv(v[i]);
        }
        for (size_type i = (cnt - 1) & ~(size_type) 1; i != ((size_type) 0) - 2; i -= 2) {
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrender(ofstream & out) const {
        primitive::glrender(out, v, n, cnt);
    }
};

class triangle_fan : public primitive {
public:
    typedef vector<string>::iterator::difference_type size_type;
    size_type cnt;
    vector3f * v;
    vector3f * n;

    triangle_fan(vector<string>::iterator i, vector<string>::iterator end) {
        cnt = end - i;
        v = new vector3f[cnt];
        n = new vector3f[cnt];
        for (size_type j = 0; j < cnt; ++i, ++j) {
            calcvertexandnormal(*i, v[j], n[j], j == 0 ? nullptr : &n[j - 1]);
        }
    }

    triangle_fan(vector<vector3f> vs, vector<vector3f> ns) {
        cnt = min(vs.size(), ns.size());
        v = new vector3f[cnt];
        n = new vector3f[cnt];
        copy(vs.begin(), vs.begin() + cnt, v);
        copy(ns.begin(), ns.begin() + cnt, n);
    }

    virtual ~triangle_fan() {
        cout << "deleting triangle_fan@" << this << endl;
        delete [] v;
    }

    virtual primitivetype type() const {
        return primitive::triangle_fan;
    }

    virtual vector3f getsizefromorigin() const {
        float ret[3] = {0, 0, 0};
        for (size_type i = 0; i < cnt; ++i) {
            ret[0] = max(abs(v[i][0]), ret[0]);
            ret[1] = max(abs(v[i][1]), ret[1]);
            ret[2] = max(v[i][2], ret[2]);
        }
        ret[0] *= 2; ret[1] *= 2;
        return ret;
    }

    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const {
        vector3f cursor(x, y, 0);
        double scr[3][3];
        gluProject(v[0][0], v[0][1], v[0][2], model, proj, view, &scr[0][0], &scr[0][1], &scr[0][2]);
        gluProject(v[1][0], v[1][1], v[1][2], model, proj, view, &scr[1][0], &scr[1][1], &scr[1][2]);
        double * scr_1 = scr[1], * scr_2 = scr[2];
        for (size_type i = 2; i < cnt; ++i) {
            gluProject(v[i][0], v[i][1], v[i][2], model, proj, view, scr_2, scr_2 + 1, scr_2 + 2);
            if (triangle::isselected(cursor, vector3f(scr[0]), vector3f(scr_1), vector3f(scr_2), z)) {
                return true;
            }
            double * tmp;
            tmp = scr_1;
            scr_1 = scr_2;
            scr_2 = tmp;
        }
        return false;
    }

    virtual void glrender() const {
       glBegin(GL_TRIANGLE_FAN);
       for (size_type i = 0; i < cnt; ++i) {
           glNormal3fv(n[i]);
           glVertex3fv(v[i]);
       }
       glEnd();
    }

    virtual void glrenderselection() const {
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        for (size_type i = 0; i < cnt; ++i) {
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrender(ofstream & out) const {
        primitive::glrender(out, v, n, cnt);
    }
};

class quad_strip : public primitive {
public:
    typedef vector<string>::iterator::difference_type size_type;
    size_type cnt;
    vector3f * v;
    vector3f * n;

    quad_strip(vector<string>::iterator i, vector<string>::iterator end) {
        cnt = (end - i) & ~(size_type) 1; // vertex count is multiple of 2
        v = new vector3f[cnt];
        n = new vector3f[cnt];
        for (size_type j = 0; j < cnt; ++i, ++j) {
            calcvertexandnormal(*i, v[j], n[j], j == 0 ? nullptr : &n[j - 1]);
        }
    }

    virtual ~quad_strip() {
        cout << "deleting quad_strip@" << this << endl;
    }

    virtual primitivetype type() const {
        return primitive::quad_strip;
    }

    virtual vector3f getsizefromorigin() const {
        float ret[3] = {0, 0, 0};
        for (size_type i = 0; i < cnt; ++i) {
            ret[0] = max(abs(v[i][0]), ret[0]);
            ret[1] = max(abs(v[i][1]), ret[1]);
            ret[2] = max(v[i][2], ret[2]);
        }
        ret[0] *= 2; ret[1] *= 2;
        return ret;
    }

    virtual bool isselected(SHORT x, SHORT y, GLdouble const * model, GLdouble const * proj, GLint const * view, float & z) const {
        vector3f cursor(x, y, 0);
        double scr[4][3];
        double * scr_0 = scr[0], * scr_1 = scr[1], * scr_2 = scr[2], *scr_3 = scr[3];
        gluProject(v[0][0], v[0][1], v[0][2], model, proj, view, &scr[0][0], &scr[0][1], &scr[0][2]);
        gluProject(v[1][0], v[1][1], v[1][2], model, proj, view, &scr[1][0], &scr[1][1], &scr[1][2]);
        for (size_type i = 2; i < cnt; i += 2) {
            size_type j = i + 1;
            gluProject(v[i][0], v[i][1], v[i][2], model, proj, view, scr_3, scr_3 + 1, scr_3 + 2);
            gluProject(v[j][0], v[j][1], v[j][2], model, proj, view, scr_2, scr_2 + 1, scr_2 + 2);
            if (triangle::isselected(cursor, scr_3, scr_0, scr_1, z) ||
                triangle::isselected(cursor, scr_1, scr_2, scr_3, z)) {
                return true;
            }
            double * tmp = scr_0;
            scr_0 = scr_3;
            scr_3 = tmp;
            tmp = scr_1;
            scr_1 = scr_2;
            scr_2 = tmp;
        }
        return false;
    }

    virtual void glrender() const {
        glBegin(GL_QUAD_STRIP);
        for (size_type i = 0; i < cnt; ++i) {
            glNormal3fv(n[i]);
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrenderselection() const {
        glLineWidth(3);
        glBegin(GL_LINE_LOOP);
        for (size_type i = 1; i < cnt; i += 2) {
            glVertex3fv(v[i]);
        }
        for (size_type i = cnt - 2; i != ((size_type) 0) - 2; i -= 2) {
            glVertex3fv(v[i]);
        }
        glEnd();
    }

    virtual void glrender(ofstream & out) const {
        primitive::glrender(out, v, n, cnt);
    }
};

class polygon : public triangle_fan {
public:
    polygon(vector<string>::iterator i, vector<string>::iterator end) : triangle_fan(i, end) {}

    virtual ~polygon() {
        cout << "polygon: ";
    }

    virtual primitivetype type() const {
        return primitive::polygon;
    }
};

#endif

#endif

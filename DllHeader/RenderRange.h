#ifdef BUILD_DLL

#ifndef __DORFY_VISION__RENDER_RANGE_H__
#define __DORFY_VISION__RENDER_RANGE_H__

#include <gl/gl.h>
#include <gl/glu.h>
#include "Vector3f.h"
#include "Types.h"

class renderrange {
private:
    bool noerror;
    dword view_dist;
    vector3f range3d[2][2][2];
    void flatten(dword x, dword y, dword xto, dword yto);
public:
    class ziterator {
        friend class renderrange;
    private:
        renderrange const & rr;
        int32 mapxmax, mapymax, z;
        dword dz;
        dword view_dist;
        vector3f range2d[2][2][2];
        vector3f drange[2][2];

        vector3f trimrange[2][2][2];
        vector3f vertices[4][2];
        dword xmini[2], xmaxi[2];
        float xmin[2], xmax[2];
        dword xstart, xend;
        ziterator(renderrange const & rr, int32 mapxmax, int32 mapymax, int32 zend, dword dz, dword view_dist);
        void prepare(dword z);
    public:
        class xiterator {
            friend class ziterator;
        private:
            ziterator const & zit;
            dword x, blocksize;
            dword currlefti[2], currrighti[2];
            float currlefty[2], currrighty[2];
            float currleftdy[2], currrightdy[2];
            dword ystart, yend;
            xiterator(ziterator const & zit, dword xstart, dword blocksize);
            void prepare();
        public:
            class yiterator {
                friend class xiterator;
            private:
                xiterator const & xit;
                dword y;
                yiterator(xiterator const & xit, dword ystart);
            public:
                void operator ++();
                bool operator <=(dword yend) const;
                dword X() const;
                dword Y() const;
                dword Z() const;
            };

            void operator ++();
            bool operator <=(dword xend) const;
            yiterator begin() const;
            dword end() const;
        };

        void operator --();
        bool operator >=(int32 zstart) const;
        xiterator begin(dword blocksize) const;
        dword end() const;

        bool contains(dword tx, dword ty, dword tz) const;
    };

    renderrange(GLdouble * matrix_model, GLdouble * matrix_proj, GLint * viewport, dword view_dist);
    bool ok() const;
    ziterator begin(int32 mapxmax, int32 mapymax, dword zend, dword dz) const;
};

#endif

#endif

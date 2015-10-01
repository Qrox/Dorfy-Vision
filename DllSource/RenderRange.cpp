#ifdef BUILD_DLL

#include "RenderRange.h"
#include "Utils.h"

/* iterate visible tiles */
void renderrange::flatten(dword x, dword y, dword xto, dword yto) {
    vector3f a = range3d[x][y][0] - range3d[xto][yto][1],
             b = range3d[x][y][1] - range3d[xto][yto][1];
    vector3f flat = (b * a[2] - a * b[2]).normalize() * view_dist;
    range3d[x][y][0] = range3d[xto][yto][0] + flat;
    range3d[x][y][1] = range3d[xto][yto][1] + flat;
}

renderrange::renderrange(GLdouble * matrix_model, GLdouble * matrix_proj, GLint * viewport, dword view_dist) : view_dist(view_dist) {
    GLint scrrange[2][2] = {viewport[0], viewport[1], viewport[0] + viewport[2], viewport[1] + viewport[3]};
    dword x0, y0, x1, y1, cnt = 0;
    for (dword x = 0; x < 2; ++x) {
        for (dword y = 0; y < 2; ++y) {
            for (dword z = 0; z < 2; ++z) {
                double tmpx, tmpy, tmpz;
                gluUnProject(scrrange[x][0], scrrange[y][1], z, matrix_model, matrix_proj, viewport, &tmpx, &tmpy, &tmpz);
                range3d[x][y][z] = vector3f(tmpx, tmpy, tmpz);
            }
            if (range3d[x][y][1][2] >= range3d[x][y][0][2]) {
                if (cnt == 0) {
                    x0 = x; y0 = y;
                } else if (cnt == 1) {
                    x1 = x; y1 = y;
                }
                ++cnt;
            }
        }
    }
    if (cnt == 2 && (x0 ^ y0 ^ x1 ^ y1)) {
        flatten(x0, y0, x1 ^ 1, y1 ^ 1);
        flatten(x1, y1, x0 ^ 1, y0 ^ 1);
        cnt = 0;
    }
    noerror = cnt == 0;
}

bool renderrange::ok() const {
    return noerror;
}

renderrange::ziterator renderrange::begin(int32 mapxmax, int32 mapymax, dword zend, dword dz) const {
    return ziterator(*this, mapxmax, mapymax, zend, dz, view_dist);
}

void renderrange::ziterator::prepare(dword z) {
    float scrleftdist = (range2d[0][1][z] - range2d[0][0][z]).mod(),
          scrrightdist = (range2d[1][1][z] - range2d[1][0][z]).mod();
    trimrange[0][0][z] = range2d[0][0][z];
    trimrange[1][0][z] = range2d[1][0][z];
    trimrange[0][1][z] = scrleftdist > view_dist ?
            range2d[0][0][z] + (range2d[0][1][z] - range2d[0][0][z]) * (view_dist / scrleftdist) :
            range2d[0][1][z];
    trimrange[1][1][z] = scrrightdist > view_dist ?
            range2d[1][0][z] + (range2d[1][1][z] - range2d[1][0][z]) * (view_dist / scrrightdist) :
            range2d[1][1][z];
    vertices[0][z] = trimrange[0][0][z];
    vertices[2][z] = trimrange[1][1][z];
    vector3f side0 = trimrange[0][1][z] - trimrange[0][0][z],
             side1 = trimrange[1][0][z] - trimrange[0][0][z];
    if (side0.cross(side1)[2] > 0) { // side0 at left
       vertices[1][z] = trimrange[1][0][z];
       vertices[3][z] = trimrange[0][1][z];
    } else { // side1 at left
       vertices[1][z] = trimrange[0][1][z];
       vertices[3][z] = trimrange[1][0][z];
    }
    xmini[z] = 0;
    xmaxi[z] = 0;
    xmin[z] = vertices[0][z][0];
    xmax[z] = vertices[0][z][0];
    for (dword i = 1; i < 4; ++i) {
       if (vertices[i][z][0] < xmin[z]) {
           xmin[z] = vertices[i][z][0];
           xmini[z] = i;
       } else if (vertices[i][z][0] > xmax[z]) {
           xmax[z] = vertices[i][z][0];
           xmaxi[z] = i;
       }
    }
}

renderrange::ziterator::ziterator(renderrange const & rr, int32 mapxmax, int32 mapymax, int32 zend, dword dz, dword view_dist)
        : rr(rr), mapxmax(mapxmax), mapymax(mapymax), z(zend), dz(dz), view_dist(view_dist) {
    for (dword x = 0; x < 2; ++x) {
        for (dword y = 0; y < 2; ++y) {
            vector3f tmp0 = rr.range3d[x][y][0], dtmp = rr.range3d[x][y][1] - tmp0;
            drange[x][y] = dtmp / dtmp[2];
            range2d[x][y][1] = tmp0 + drange[x][y] * (z + 1 - tmp0[2]);
            range2d[x][y][0] = range2d[x][y][1] - drange[x][y] * dz;
        }
    }
    prepare(1);
    prepare(0);
    xstart = (dword) limit(0, (int32) (min(xmin[0], xmin[1]) + .5), mapxmax);
    xend = (dword) limit(0, (int32) (max(xmax[0], xmax[1]) + .5), mapxmax);
}

void renderrange::ziterator::operator --() {
    z -= dz;
    for (dword x = 0; x < 2; ++x) {
        for (dword y = 0; y < 2; ++y) {
            range2d[x][y][1] = range2d[x][y][0];
            range2d[x][y][0] = range2d[x][y][1] - drange[x][y] * dz;
            trimrange[x][y][1] = trimrange[x][y][0];
        }
    }
    for (dword i = 0; i < 4; ++i) {
        vertices[i][1] = vertices[i][0];
    }
    xmini[1] = xmini[0];
    xmaxi[1] = xmaxi[0];
    xmin[1] = xmin[0];
    xmax[1] = xmax[0];
    prepare(0);
    xstart = (dword) limit(0, (int32) (min(xmin[0], xmin[1]) + .5), mapxmax);
    xend = (dword) limit(0, (int32) (max(xmax[0], xmax[1]) + .5), mapxmax);
}

bool renderrange::ziterator::operator >=(int32 zstart) const {
    return z >= zstart;
}

renderrange::ziterator::xiterator renderrange::ziterator::begin(dword blocksize) const {
    return xiterator(*this, xstart - xstart % blocksize, blocksize);
}

dword renderrange::ziterator::end() const {
    return xend;
}

//todo
bool renderrange::ziterator::contains(dword tx, dword ty, dword tz) const {
    if ((int32) tz > z - (int32) dz && (int32) tz <= z) {
        xiterator x(*this, tx, 1);
        return x.ystart <= ty && ty <= x.yend;
    } else {
        return false;
    }
}

void renderrange::ziterator::xiterator::prepare() {
    float leftymin[2], rightymax[2];
    for (dword z = 0; z < 2; ++z) {
        bool leftchanged = false, rightchanged = false;
        bool leftyminfound = false, rightymaxfound = false;
        while (currlefti[z] != zit.xmaxi[z] && zit.vertices[currlefti[z]][z][0] <= x + blocksize - .5) { // max x of tile/block exceeds max x of current edge
            dword nextlefti = (currlefti[z] - 1) & 3;
            vector3f nextleftedge = zit.vertices[nextlefti][z] - zit.vertices[currlefti[z]][z];
            float nextleftdy = nextleftedge[1] / nextleftedge[0];
            if (currleftdy[z] <= 0 && nextleftdy > 0) { // turn point. <= 0 because init value of currdlefty == 0
                leftyminfound = true;
                leftymin[z] = zit.vertices[currlefti[z]][z][1];
            }
            currlefti[z] = nextlefti;
            currleftdy[z] = nextleftdy;
            leftchanged = true;
        }
        while (currrighti[z] != zit.xmaxi[z] && zit.vertices[currrighti[z]][z][0] <= x + blocksize - .5) {
            dword nextrighti = (currrighti[z] + 1) & 3;
            vector3f nextrightedge = zit.vertices[nextrighti][z] - zit.vertices[currrighti[z]][z];
            float nextrightdy = nextrightedge[1] / nextrightedge[0];
            if (currrightdy[z] >= 0 && nextrightdy < 0) {
                rightymaxfound = true;
                rightymax[z] = zit.vertices[currrighti[z]][z][1];
            }
            currrighti[z] = nextrighti;
            currrightdy[z] = nextrightdy;
            rightchanged = true;
        }
        float nextlefty, nextrighty;
        if (leftchanged) { // need to recalc intersection point
            dword lastlefti = (currlefti[z] + 1) & 3;
            nextlefty = zit.vertices[lastlefti][z][1] + currleftdy[z] * (x + blocksize - .5 - zit.vertices[lastlefti][z][0]);
        } else { // approach currlefti by dx = 1
            nextlefty = currlefty[z] + currleftdy[z] * blocksize;
        }
        if (rightchanged) {
            dword lastrighti = (currrighti[z] - 1) & 3;
            nextrighty = zit.vertices[lastrighti][z][1] + currrightdy[z] * (x + blocksize - .5 - zit.vertices[lastrighti][z][0]);
        } else {
            nextrighty = currrighty[z] + currrightdy[z] * blocksize;
        }
        if (!leftyminfound) { // no turn point
            if (currleftdy[z] <= 0) { // all toward -y
                leftymin[z] = nextlefty;
            } else { // all toward +y
                leftymin[z] = currlefty[z];
            }
        }
        if (!rightymaxfound) {
            if (currrightdy[z] >= 0) { // all toward +y
                rightymax[z] = nextrighty;
            } else { // all toward -y
                rightymax[z] = currrighty[z];
            }
        }
        currlefty[z] = nextlefty;
        currrighty[z] = nextrighty;
    }
    ystart = (dword) limit(0, (int32) (min(leftymin[0], leftymin[1]) + .5), zit.mapymax);
    yend = (dword) limit(0, (int32) (max(rightymax[0], rightymax[1]) + .5), zit.mapymax);
}

renderrange::ziterator::xiterator::xiterator(ziterator const & zit, dword xstart, dword blocksize) : zit(zit), x(xstart), blocksize(blocksize) {
    for (dword h = 0; h < 2; ++h) {
        currlefti[h] = zit.xmini[h];
        currrighti[h] = zit.xmini[h]; // currently approaching vertex
        currlefty[h] = zit.vertices[currlefti[h]][h][1];
        currrighty[h] = zit.vertices[currrighti[h]][h][1];
        currleftdy[h] = 0;
        currrightdy[h] = 0;
    }
    prepare();
}

void renderrange::ziterator::xiterator::operator ++() {
    x += blocksize;
    prepare();
}

bool renderrange::ziterator::xiterator::operator <=(dword xend) const {
    return x <= xend;
}

renderrange::ziterator::xiterator::yiterator renderrange::ziterator::xiterator::begin() const {
    return yiterator(*this, ystart - ystart % blocksize);
}

dword renderrange::ziterator::xiterator::end() const {
    return yend;
}

renderrange::ziterator::xiterator::yiterator::yiterator(xiterator const & xit, dword ystart) : xit(xit), y(ystart) {}

void renderrange::ziterator::xiterator::yiterator::operator ++() {
    y += xit.blocksize;
}

bool renderrange::ziterator::xiterator::yiterator::operator <=(dword yend) const {
    return y <= yend;
}

dword renderrange::ziterator::xiterator::yiterator::X() const {
    return xit.x;
}

dword renderrange::ziterator::xiterator::yiterator::Y() const {
    return y;
}

dword renderrange::ziterator::xiterator::yiterator::Z() const {
    return xit.zit.z;
}

#endif

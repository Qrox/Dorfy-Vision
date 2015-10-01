#ifdef BUILD_MODEL

#ifndef __DORFY_VISION__MODEL__GRAPHICS_H__
#define __DORFY_VISION__MODEL__GRAPHICS_H__

#include <windows.h>
#include <gl/gl.h>
#include "Vector3f.h"

extern HDC hdc;

extern float eye_distance, scale;
extern vector3f stance;
extern vector3f eye;
extern GLfloat lightpos[4];

extern GLdouble matrix_frustum[16], matrix_ortho[16], matrix_modelview3d[16];
extern GLint gl_viewport[4];
extern bool frustum;

int glInit(HWND window);
bool glRepaint();
void glResize(GLsizei width, GLsizei height);

#endif

#endif

#ifdef BUILD_DLL

#ifndef __DORFY_VISION__GRAPHICS_H__
#define __DORFY_VISION__GRAPHICS_H__

#include <windows.h>
#include <gl/gl.h>
#include "Shader.h"

extern HDC hdc, hud, hdc_text;
extern HGLRC hrc;
extern HFONT hud_font, text_font;
extern HBITMAP hud_bitmap, text_bitmap;
extern float center_x, center_y, center_z;
extern int32 const view_radius, view_depth;
extern float view_angle, pitch_angle;

extern shader shader_vert, shader_frag;
extern shaderprogram shader_prog;
extern GLint shader_fog, shader_start, shader_k;

int glInit(HWND mainWindow);
void glResize(GLsizei width, GLsizei height);
bool glRepaint();
void hudRepaint();

#endif

#endif

#ifdef BUILD_MODEL

#include <cmath>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glext.h>
#include <windows.h>
#include "MainWindow.h"
#include "Graphics.h"
#include "Types.h"
typedef int64 off64_t;
#include <io.h>

#undef M_PI
double constexpr M_PI = 3.14159265358979323846;

using namespace std;

// fix eclipse weird bug
GLAPI void APIENTRY gluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ);

HDC hdc = 0;

HDC hdc_text = 0;
HFONT text_font = 0;
HBITMAP text_bitmap = 0;
void * painted_text = 0;
GLint texture_max_size, text_bitmap_width, text_bitmap_height, text_font_size;

#define ENABLE_LIGHT 1

int glInit(HWND window) {
    hdc = GetDC(window);
    PIXELFORMATDESCRIPTOR pfd = {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,
        32,
        0, 0, 0, 0, 0, 0, 0, 0,
        0,
        0, 0, 0, 0,
        16,
        0,
        0,
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };
    GLuint pixel_format = ChoosePixelFormat(hdc, &pfd);
    SetPixelFormat(hdc, pixel_format, &pfd);
    HGLRC hrc = wglCreateContext(hdc);
    wglMakeCurrent(hdc, hrc);

    RECT rect;
    if (GetClientRect(modelWindow, &rect)) {
        glResize(rect.right - rect.left, rect.bottom - rect.top);
    }

    if (frustum) {
        glEnable(GL_CULL_FACE);
    }
    glCullFace(GL_BACK);
    glEnable(GL_RESCALE_NORMAL); // prevents that changing the scale also changes brightness (the scaling matrix also scales the normals)
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
#if ENABLE_LIGHT
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
#endif
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &texture_max_size);
    if (texture_max_size < 4096) {
        cout << "The max texture size supported by your OpenGL implementation is less than 4096. You may experience very obscure text etc." << endl;
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    LOGFONT font;
    font.lfWidth = 0;
    font.lfEscapement = 0;
    font.lfOrientation = 0;
    font.lfWeight = FW_NORMAL;
    font.lfItalic = 0;
    font.lfUnderline = 0;
    font.lfStrikeOut = 0;
    font.lfOutPrecision = OUT_TT_PRECIS;
    font.lfPitchAndFamily = DEFAULT_PITCH || FF_DONTCARE;
    font.lfQuality = ANTIALIASED_QUALITY;

    BITMAPINFO bminfo;
    bminfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bminfo.bmiHeader.biPlanes = 1;
    bminfo.bmiHeader.biBitCount = 32;
    bminfo.bmiHeader.biCompression = BI_RGB;
    bminfo.bmiHeader.biSizeImage = 0;
    bminfo.bmiHeader.biXPelsPerMeter = 0;
    bminfo.bmiHeader.biYPelsPerMeter = 0;
    bminfo.bmiHeader.biClrUsed = 0;
    bminfo.bmiHeader.biClrImportant = 0;
    bminfo.bmiColors[0].rgbBlue = 255;
    bminfo.bmiColors[0].rgbGreen = 255;
    bminfo.bmiColors[0].rgbRed = 255;
    bminfo.bmiColors[0].rgbReserved = 255;

    hdc_text = CreateCompatibleDC(NULL);
    SetBkMode(hdc_text, TRANSPARENT);
    text_font_size = min(32, texture_max_size / 80);
    font.lfHeight = text_font_size;
    text_font = CreateFontIndirect(&font);
    DeleteObject(SelectObject(hdc_text, text_font));
    text_bitmap_width = texture_max_size;
    text_bitmap_height = 1;
    for (int fs = text_font_size; fs; fs >>= 1, text_bitmap_height <<= 1) {
    }
    if (text_bitmap_height == text_font_size * 2) {
        text_bitmap_height = text_font_size;
    }
    bminfo.bmiHeader.biWidth = text_bitmap_width;
    bminfo.bmiHeader.biHeight = -text_bitmap_height;
    text_bitmap = CreateDIBSection(0, &bminfo, DIB_RGB_COLORS, &painted_text, NULL, 0);
    DeleteObject(SelectObject(hdc_text, text_bitmap));

    return 1;
}

float constexpr viewport_size = .293;
float scrPorpotion = 1;

GLdouble matrix_frustum[16], matrix_ortho[16], matrix_modelview3d[16];
GLint gl_viewport[4];
bool frustum = false;

void glResize(GLsizei width, GLsizei height) {
    float constexpr angleofview = M_PI / 3;
    float constexpr rd = .5 / tan(angleofview / 2);

    glViewport(0, 0, width, height);
    glGetIntegerv(GL_VIEWPORT, gl_viewport);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    scrPorpotion = (width == 0 || height == 0) ? 1 : (float) width / (float) height;
    float w, h, d;
    if (width <= height) {
        w = viewport_size;
        h = w / scrPorpotion;
        d = w * rd;
        if (frustum) {
            float _d = d / 0.001;
            w /= _d;
            h /= _d;
            d /= _d;
            glFrustum(-w, w, -h, h, d, 5);
        } else {
            float _d = w / 2;
            w /= _d;
            h /= _d;
            d = 0.001;
            glOrtho(-w, w, -h, h, d, 5);
        }
        glTranslatef(0, 0, -d);
    } else {
        h = viewport_size;
        w = h * scrPorpotion;
        d = h * rd;
        if (frustum) {
            float _d = d / 0.001;
            w /= _d;
            h /= _d;
            d /= _d;
            glFrustum(-w, w, -h, h, d, 5);
        } else {
            float _d = h / 2;
            w /= _d;
            h /= _d;
            d = 0.001;
            glOrtho(-w, w, -h, h, d, 5);
        }
        glTranslatef(0, 0, -d);
    }
    glGetDoublev(GL_PROJECTION_MATRIX, matrix_frustum);
    glLoadIdentity();
    h = 1;
    w = h * scrPorpotion;
    glOrtho(0, w, h, 0, 0, 1);
    glGetDoublev(GL_PROJECTION_MATRIX, matrix_ortho);
}

struct TEXT_SIZE {float texW, texH, strRatio;};

vector3f glTextSize(string text, float height) {
    SIZE text_size;
    bool got_size = GetTextExtentPoint32(hdc_text, text.data(), text.length(), &text_size);
    if (got_size) {
        return vector3f(height * ((float) text_size.cx / text_size.cy), height, 0);
    } else {
        return vector3f(height, height, 0);
    }
}

TEXT_SIZE glText(char const * text, int len) {
    SIZE text_size;
    bool got_size = GetTextExtentPoint32(hdc_text, text, len, &text_size);
//  if (text_size.cx > text_bitmap_size || text_size.cy > text_bitmap_size) {
//      prompt_texture_overflow();
//  }
    dword texture_size = text_bitmap_width * text_bitmap_height;
    dword * texture_data = (dword *) painted_text;
    fill_n(texture_data, texture_size, 0xFF000000);
    SetTextColor(hdc_text, 0x00FFFFFF);
    TextOut(hdc_text, 0, 0, text, len);
    while (texture_size) {
        *texture_data ^= 0xFF000000;
        ++texture_data;
        --texture_size;
    }
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, text_bitmap_width, text_bitmap_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, painted_text);
    if (got_size) {
        return {(float) text_size.cx / text_bitmap_width, (float) text_size.cy / text_bitmap_height, (float) text_size.cx / text_size.cy};
    } else {
        return {1, 1, (float) text_bitmap_width / text_bitmap_height};
    }
}

enum class text_align {
    left, right, center
};

void glQuadText(char const * text, int len, float x, float y, float height, text_align align) {
    TEXT_SIZE size = glText(text, len);
    float width = height * size.strRatio;
    float l = x, r = x, t = y, b = y + height;
    switch (align) {
    case text_align::left:
        r += width;
        break;
    case text_align::right:
        l -= width;
        break;
    case text_align::center:
        l -= width / 2;
        r += width / 2;
        break;
    }
    glBegin(GL_QUADS);
    glTexCoord2f(0, 0);                 glVertex2f(l, t);
    glTexCoord2f(0, size.texH);         glVertex2f(l, b);
    glTexCoord2f(size.texW, size.texH); glVertex2f(r, b);
    glTexCoord2f(size.texW, 0);         glVertex2f(r, t);
    glEnd();
}

inline void glQuadText(string const & s, float x, float y, float height, text_align align) {
    glQuadText(s.data(), s.length(), x, y, height, align);
}

GLvoid glLiquidTest() {
    float constexpr scale = .5;
    glScalef(scale, scale, scale);
    float constexpr unit_liquid = 1. / 8.;
    int8 map[16][16] = {
       //0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //0
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //1
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //2
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //3
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //4
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //5
        {0, 0, 0, 0, 0, 0,-1,-1,-1,-1, 0, 0, 0, 0, 0, 0}, //6
        {0, 0, 0, 0, 0, 0,-1, 7, 7, 7, 0, 0, 0, 0, 0, 0}, //7
        {0, 0, 0, 0, 0, 0,-1, 7, 7, 7, 0, 0, 0, 0, 0, 0}, //8
        {0, 0, 0, 0, 0, 0,-1, 7, 7, 7, 0, 0, 0, 0, 0, 0}, //9
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //A
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //B
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //C
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //D
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //E
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, //F
    };
    for (dword i = 1; i < 15; ++i) {
        for (dword j = 1; j < 15; ++j) {
            int8 liquid_level[3][3];
            for (dword x = 0; x < 3; ++x) {
                for (dword y = 0; y < 3; ++y) {
                    liquid_level[x][y] = map[j + y - 1][i + x - 1];
                }
            }
            if (liquid_level[1][1] > 0) {
                float grid[5][5];
                for (dword x = 0; x < 3; ++x) {
                    for (dword y = 0; y < 3; ++y) {
                        if (liquid_level[x][y] > 0) {
                            grid[x * 2][y * 2] = liquid_level[x][y] * unit_liquid;
                        } else {
                            grid[x * 2][y * 2] = 0;
                        }
                    }
                }
                for (dword x = 0; x < 2; ++x) {
                    for (dword y = 0; y < 2; ++y) {
                        if (liquid_level[x][y] == 0 || liquid_level[x][y + 1] == 0 ||
                            liquid_level[x + 1][y] == 0 || liquid_level[x + 1][y + 1] == 0) {
                            grid[x * 2 + 1][y * 2 + 1] = 0;
                        } else {
                            dword cnt = 0, sum = 0;
                            if (liquid_level[x][y] > 0) {++cnt; sum += liquid_level[x][y];}
                            if (liquid_level[x][y + 1] > 0) {++cnt; sum += liquid_level[x][y + 1];}
                            if (liquid_level[x + 1][y] > 0) {++cnt; sum += liquid_level[x + 1][y];}
                            if (liquid_level[x + 1][y + 1] > 0) {++cnt; sum += liquid_level[x + 1][y + 1];}
                            if (cnt == 0) {
                                grid[x * 2 + 1][y * 2 + 1] = 0;
                            } else {
                                grid[x * 2 + 1][y * 2 + 1] = unit_liquid * sum / cnt;
                            }
                        }
                    }
                }
                for (dword x = 0; x < 2; ++x) {
                    for (dword y = 0; y < 3; ++y) {
                        if (liquid_level[x][y] == 0 || liquid_level[x + 1][y] == 0) {
                            grid[x * 2 + 1][y * 2] = 0;
                        } else {
                            dword cnt = 0, sum = 0;
                            if (liquid_level[x][y] > 0) {++cnt; sum += liquid_level[x][y];}
                            if (liquid_level[x + 1][y] > 0) {++cnt; sum += liquid_level[x + 1][y];}
                            if (cnt == 0) {
                                grid[x * 2 + 1][y * 2] = 0;
                            } else {
                                grid[x * 2 + 1][y * 2] = unit_liquid * sum / cnt;
                            }
                        }
                    }
                }
                for (dword x = 0; x < 3; ++x) {
                    for (dword y = 0; y < 2; ++y) {
                        if (liquid_level[x][y] == 0 || liquid_level[x][y + 1] == 0) {
                            grid[x * 2][y * 2 + 1] = 0;
                        } else {
                            dword cnt = 0, sum = 0;
                            if (liquid_level[x][y] > 0) {++cnt; sum += liquid_level[x][y];}
                            if (liquid_level[x][y + 1] > 0) {++cnt; sum += liquid_level[x][y + 1];}
                            if (cnt == 0) {
                                grid[x * 2][y * 2 + 1] = 0;
                            } else {
                                grid[x * 2][y * 2 + 1] = unit_liquid * sum / cnt;
                            }
                        }
                    }
                }
                vector3f normals[3][3];
                normals[1][1] = vector3f(
                    grid[1][1] / 2 + grid[1][2] + grid[1][3] / 2 - grid[3][1] / 2 - grid[3][2] - grid[3][3] / 2,
                    grid[1][1] / 2 + grid[2][1] + grid[3][1] / 2 - grid[1][3] / 2 - grid[2][3] - grid[3][3] / 2,
                    2).normalize();
                for (dword x = 0; x < 2; ++x) {
                    for (dword y = 0; y < 2; ++y) {
                        float nx = 0, ny = 0, nz = 0;
                        for (dword a = 0; a < 2; ++a) {
                            for (dword b = 0; b < 2; ++b) {
                                if (liquid_level[x + a][y + b] > 0) {
                                    nx += (grid[x * 2 + a][y * 2 + b] + grid[x * 2 + a][y * 2 + b + 1] - grid[x * 2 + a + 1][y * 2 + b] - grid[x * 2 + a + 1][y * 2 + b + 1]) / 2;
                                    ny += (grid[x * 2 + a][y * 2 + b] + grid[x * 2 + a + 1][y * 2 + b] - grid[x * 2 + a][y * 2 + b + 1] - grid[x * 2 + a + 1][y * 2 + b + 1]) / 2;
                                    nz += .5;
                                }
                            }
                        }
                        normals[x * 2][y * 2] = vector3f(nx, ny, nz).normalize();
                    }
                }
                for (dword x = 0; x < 2; ++x) {
                    float nx = 0, ny = 0, nz = 0;
                    for (dword a = 0; a < 2; ++a) {
                        if (liquid_level[x + a][1] > 0) {
                            nx += grid[x * 2 + a][2] - grid[x * 2 + a + 1][2];
                            ny += (grid[x * 2 + 1][1] - grid[x * 2 + 1][3]) / 2;
                            nz += .5;
                        }
                    }
                    normals[x * 2][1] = vector3f(nx, ny, nz).normalize();
                }
                for (dword y = 0; y < 2; ++y) {
                    float nx = 0, ny = 0, nz = 0;
                    for (dword b = 0; b < 2; ++b) {
                        if (liquid_level[1][y + b] > 0) {
                            nx += (grid[1][y * 2 + 1] - grid[3][y * 2 + 1]) / 2;
                            ny += grid[2][y * 2 + b] - grid[2][y * 2 + b + 1];
                            nz += .5;
                        }
                    }
                    normals[1][y * 2] = vector3f(nx, ny, nz).normalize();
                }
                glPushMatrix();
                glTranslatef(i - 7.5, j - 7.5, 0);
                glColor3f(.5, .5, .5);
                glBegin(GL_TRIANGLE_FAN);
                glNormal3fv(normals[1][1]);
                glVertex3f(0, 0, grid[2][2]);
                glNormal3fv(normals[2][1]);
                glVertex3f(.5, 0, grid[3][2]);
                glNormal3fv(normals[2][0]);
                glVertex3f(.5, -.5, grid[3][1]);
                glNormal3fv(normals[1][0]);
                glVertex3f(0, -.5, grid[2][1]);
                glNormal3fv(normals[0][0]);
                glVertex3f(-.5, -.5, grid[1][1]);
                glNormal3fv(normals[0][1]);
                glVertex3f(-.5, 0, grid[1][2]);
                glNormal3fv(normals[0][2]);
                glVertex3f(-.5, .5, grid[1][3]);
                glNormal3fv(normals[1][2]);
                glVertex3f(0, .5, grid[2][3]);
                glNormal3fv(normals[2][2]);
                glVertex3f(.5, .5, grid[3][3]);
                glNormal3fv(normals[2][1]);
                glVertex3f(.5, 0, grid[3][2]);
                glEnd();
                glBegin(GL_LINES);
                glColor3f(1, 0, 0);
                glVertex3f(0, 0, grid[2][2]);
                glColor3f(0, 1, 0);
                glVertex3fv(normals[1][1] * .1 + vector3f(0, 0, grid[2][2]));
                glColor3f(1, 0, 0);
                glVertex3f(.5, 0, grid[3][2]);
                glColor3f(0, 1, 0);
                glVertex3fv(normals[2][1] * .1 + vector3f(.5, 0, grid[3][2]));
                glColor3f(1, 0, 0);
                glVertex3f(.5, -.5, grid[3][1]);
                glColor3f(0, 1, 0);
                glVertex3fv(normals[2][0] * .1 + vector3f(.5, -.5, grid[3][1]));
                glColor3f(1, 0, 0);
                glVertex3f(0, -.5, grid[2][1]);
                glColor3f(0, 1, 0);
                glVertex3fv(normals[1][0] * .1 + vector3f(0, -.5, grid[2][1]));
                glColor3f(1, 0, 0);
                glVertex3f(-.5, -.5, grid[1][1]);
                glColor3f(0, 1, 0);
                glVertex3fv(normals[0][0] * .1 + vector3f(-.5, -.5, grid[1][1]));
                glColor3f(1, 0, 0);
                glVertex3f(-.5, 0, grid[1][2]);
                glColor3f(0, 1, 0);
                glVertex3fv(normals[0][1] * .1 + vector3f(-.5, 0, grid[1][2]));
                glColor3f(1, 0, 0);
                glVertex3f(-.5, .5, grid[1][3]);
                glColor3f(0, 1, 0);
                glVertex3fv(normals[0][2] * .1 + vector3f(-.5, .5, grid[1][3]));
                glColor3f(1, 0, 0);
                glVertex3f(0, .5, grid[2][3]);
                glColor3f(0, 1, 0);
                glVertex3fv(normals[1][2] * .1 + vector3f(0, .5, grid[2][3]));
                glColor3f(1, 0, 0);
                glVertex3f(.5, .5, grid[3][3]);
                glColor3f(0, 1, 0);
                glVertex3fv(normals[2][2] * .1 + vector3f(.5, .5, grid[3][3]));
                glEnd();
                glPopMatrix();
            }
        }
    }
}

#define MODEL_OUTPUT 0

enum glFunction : dword {
    vertex,
    normal
};

struct command {
    glFunction function;
    float value[3];
};

#if !MODEL_OUTPUT

class Model {
public:
    byte * data, * end;

    Model(string file) {
        int hfile = open(file.data(), 0x0100);
        long length = filelength(hfile);
        if (length < 0 || length >= 0x10000) {
            cout << "model file too large!" << endl;
            data = nullptr;
            end = nullptr;
        } else {
            dword len = length;
            data = new byte[len];
            end = data + len;
            if ((dword) read(hfile, data, len) != len) {
                cout << "fail to complete model loading!" << endl;
                data = nullptr;
                end = nullptr;
            }
        }
        close(hfile);
    }

    ~Model() {
        delete [] data;
    }

    void glRender() {
        byte * data = this->data, * end = this->end;
        while (((word *) data) + 2 <= (word *) end) {
            word type = *(word *) data; data += sizeof(word);
            word size = *(word *) data; data += sizeof(word);
            if (((command *) data) + size <= (command *) end) {
                glBegin(type);
                while (size--) {
                    switch (((command *) data)->function) {
                    case vertex:
                        glVertex3fv(((command *) data)->value);
                        break;
                    case normal:
                        glNormal3fv(((command *) data)->value);
                        break;
                    }
                    data += sizeof(command);
                }
                glEnd();
            }
        }
    }
};

#endif

float eye_distance = sqrt(3), scale = 1;
vector3f stance(0, 0, 1);
vector3f eye(0, 1, 0);
GLfloat lightpos[4] = {1, 1, 1, 0};

bool glRepaint() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 3d
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(matrix_frustum);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
//    gluLookAt(1, 1, 1.5, 0, 0, .5, -1, -1, 1);

//    gluLookAt(eye[0] * eye_distance, eye[1] * eye_distance, eye[2] * eye_distance + .5,
//              0, 0, .5,
//              stance[0], stance[1], stance[2]
//    );

    gluLookAt(eye[1] * eye_distance, eye[0] * eye_distance, eye[2] * eye_distance + .5,
              0, 0, .5,
              stance[1], stance[0], stance[2]
    );
    static float constexpr lefthanded[] = {
        0, 1, 0, 0,
        1, 0, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1,
    };
    glMultMatrixf(lefthanded);
    glScalef(scale, scale, scale);
    glGetDoublev(GL_MODELVIEW_MATRIX, matrix_modelview3d);

#if ENABLE_LIGHT
    glDisable(GL_LIGHTING);
#endif

    //axes
    glLineWidth(1);
    glBegin(GL_LINES);
    glColor3f(1, 0, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(1, 0, 0);
    glColor3f(0, 1, 0);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 1, 0);
    glColor3f(0, 0, 1);
    glVertex3f(0, 0, 0);
    glVertex3f(0, 0, 1);
    glEnd();

#if MODEL_OUTPUT
    GLfloat glModel();
    glModel();
#else

    glColor3f(1, 1, 0);
    if (selectionindex < selectedprimitives.size()) {
        selectedprimitives[selectionindex].prim->glrenderselection();
    }

#if ENABLE_LIGHT
    glEnable(GL_LIGHTING);
    dword ticks_of_day = 450;
    GLfloat lightamb[3], lightdif[3], lightspe[3];
    float32 angle, brightness;
    if (ticks_of_day < 300 || ticks_of_day > 900) { // night, moon
        angle = (ticks_of_day < 300 ? ticks_of_day + 300 : ticks_of_day - 900) / 600. * M_PI;
        brightness = sin(angle) * .1;
    } else { // day, sun
        angle = (ticks_of_day - 300) / 600. * M_PI;
        brightness = sin(angle);
    }
    float32 red_enhancement = 0;
    if (angle < M_PI / 9) {
        red_enhancement = sin(angle * 9) * .1;
    } else if (angle > M_PI * 8 / 9) {
        red_enhancement = sin((M_PI - angle) * 9) * .1;
    }
    lightamb[0] = .10;
    lightamb[1] = .05 + brightness * .1;
    lightamb[2] = .04 + brightness * .16;
    lightdif[0] = brightness * (.9 + red_enhancement);
    lightdif[1] = brightness * .85;
    lightdif[2] = brightness * .8;
    lightspe[0] = brightness * (.9 + red_enhancement);
    lightspe[1] = brightness * .85;
    lightspe[2] = brightness * .8;
    glLightfv(GL_LIGHT0, GL_POSITION, lightpos);
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightamb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightdif);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightspe);
#endif

    glColor3f(1, 1, 1);
    for (auto i = primitives.begin(); i != primitives.end(); ++i) {
        (**i).glrender();
    }

#endif

    // text
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixd(matrix_ortho);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPushAttrib(GL_ENABLE_BIT);
    glDisable(GL_DEPTH_TEST);
#if ENABLE_LIGHT
    glDisable(GL_LIGHTING);
#endif
    float constexpr h = .03, x = 0, y = 1 - h;
    glColor3f(1, 1, 1);
    if (!cmdline.empty()) {
        glEnable(GL_TEXTURE_2D);
        glQuadText(cmdline, x, y, h, text_align::left);
        glDisable(GL_TEXTURE_2D);
    }
    float cx;
    if (cmdcursor == 0) {
        cx = x;
    } else if (cmdcursor < cmdline.length()) {
        vector3f cur = glTextSize(cmdline.substr(0, cmdcursor), h);
        cx = x + cur[0];
    } else {
        vector3f cur = glTextSize(cmdline, h);
        cx = x + cur[0];
    }
    if (!cmdline.empty() || !lock_mouse) {
        glLineWidth(2);
        glBegin(GL_LINES);
        glVertex2f(cx, y);
        glVertex2f(cx, y + h);
        glEnd();
        glLineWidth(1);
    }
    glPopAttrib();
    return true;
}

#if MODEL_OUTPUT
#include <vector>

ofstream model;
bool model_first = true;
bool began = false;
vector<command> commands;

void glBeginOut(word type) {
    if (model_first) {
        if (began) {
            cout << "begin after already began!" << endl;
        } else {
            model.write((char *) &type, 2);
            commands.clear();
            began = true;
        }
    }
    glBegin(type);
}

void glEndOut() {
    if (model_first) {
        if (began) {
            word size = commands.size();
            model.write((char *) &size, 2);
            for (auto i = commands.begin(); i < commands.end(); ++i) {
                model.write((char *) &*i, sizeof(command));
            }
            began = false;
        } else {
            cout << "end before begins!" << endl;
        }
    }
    glEnd();
}

void glVertex3fOut(float x, float y, float z) {
    if (model_first) {
        if (began) {
            commands.push_back({vertex, {x, y, z}});
        } else {
            cout << "vertex belonging nowhere!" << endl;
        }
    }
    glVertex3f(x, y, z);
}

void glVertex3fvOut(float const * v) {
    glVertex3fOut(v[0], v[1], v[2]);
}

void glNormal3fOut(float x, float y, float z) {
    if (model_first) {
        if (began) {
            commands.push_back({normal, {x, y, z}});
        } else {
            cout << "normal belonging nowhere!" << endl;
        }
    }
    glNormal3f(x, y, z);
}

void glNormal3fvOut(float const * v) {
    glNormal3fOut(v[0], v[1], v[2]);
}

#define glBegin(v)          glBeginOut(v)
#define glEnd()             glEndOut()
#define glVertex3f(x, y, z) glVertex3fOut(x, y, z)
#define glVertex3fv(v)      glVertex3fvOut(v)
#define glNormal3f(x, y, z) glNormal3fOut(x, y, z)
#define glNormal3fv(v)      glNormal3fvOut(v)
#endif

GLvoid glConeFrustum(vector3f const & b, vector3f const & t, float rb, float rt, dword fcnt) {
    vector3f h0(0, 0, 1);
    vector3f h = t - b;
    double lh = h.mod();
    vector3f rot = h0.angleto(h);
    glBegin(GL_TRIANGLE_FAN);
    glNormal3fv(vector3f(0, 0, -1).rotate(rot));
    for (dword i = 0; i < fcnt; ++i) {
        glVertex3fv(vector3f(cos(M_PI * 2 / fcnt * i) * rb, sin(M_PI * 2 / fcnt * i) * rb, 0).rotate(rot) + b);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3fv(vector3f(0, 0, 1).rotate(rot));
    for (int32 i = fcnt - 1; i >= 0; --i) {
        glVertex3fv(vector3f(cos(M_PI * 2 / fcnt * i) * rt, sin(M_PI * 2 / fcnt * i) * rt, lh).rotate(rot) + b);
    }
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    for (dword _i = 0; _i <= fcnt; ++_i) {
        dword i = _i % fcnt;
        vector3f eb = vector3f(cos(M_PI * 2 / fcnt * i), sin(M_PI * 2 / fcnt * i), 0).rotate(rot);
        glNormal3fv((eb * lh + vector3f(0, 0, rb - rt)).normalize());
        glVertex3fv(eb * rb + b);
        glVertex3fv(vector3f(cos(M_PI * 2 / fcnt * i) * rt, sin(M_PI * 2 / fcnt * i) * rt, lh).rotate(rot) + b);
    }
    glEnd();
}

//GLvoid glPolyFrustum(vector3f const & b, vector3f const & t, float rb, float rt, dword sidecnt) {
//    vector3f h0(0, 0, 1);
//    vector3f h = t - b;
//    double lh = h.mod();
//    vector3f rot = h0.angleto(h);
//    glBegin(GL_TRIANGLE_FAN);
//    glNormal3fv(vector3f(0, 0, -1).rotate(rot));
//    for (dword i = 0; i < sidecnt; ++i) {
//        glVertex3fv(vector3f(cos(M_PI * 2 / sidecnt * i) * rb, sin(M_PI * 2 / sidecnt * i) * rb, 0).rotate(rot) + b);
//    }
//    glEnd();
//    glBegin(GL_TRIANGLE_FAN);
//    glNormal3fv(vector3f(0, 0, 1).rotate(rot));
//    for (int32 i = sidecnt - 1; i >= 0; --i) {
//        glVertex3fv(vector3f(cos(M_PI * 2 / sidecnt * i) * rt, sin(M_PI * 2 / sidecnt * i) * rt, lh).rotate(rot) + b);
//    }
//    glEnd();
//    glBegin(GL_QUADS);
//    for (dword i = 0; i < sidecnt; ++i) {
//
//    }
//    glEnd();
//}

GLvoid glCube(float x, float y, float z) {
    float rx = x * .5, ry = y * .5;
    glBegin(GL_QUADS);
    glNormal3f(0, 0, 1);
    glVertex3f(rx, -ry, z);
    glVertex3f(-rx, -ry, z);
    glVertex3f(-rx, ry, z);
    glVertex3f(rx, ry, z);
    glNormal3f(1, 0, 0);
    glVertex3f(rx, -ry, z);
    glVertex3f(rx, ry, z);
    glVertex3f(rx, ry, 0);
    glVertex3f(rx, -ry, 0);
    glNormal3f(0, 1, 0);
    glVertex3f(rx, ry, z);
    glVertex3f(-rx, ry, z);
    glVertex3f(-rx, ry, 0);
    glVertex3f(rx, ry, 0);
    glNormal3f(-1, 0, 0);
    glVertex3f(-rx, ry, z);
    glVertex3f(-rx, -ry, z);
    glVertex3f(-rx, -ry, 0);
    glVertex3f(-rx, ry, 0);
    glNormal3f(0, -1, 0);
    glVertex3f(-rx, -ry, z);
    glVertex3f(rx, -ry, z);
    glVertex3f(rx, -ry, 0);
    glVertex3f(-rx, -ry, 0);
    glNormal3f(0, 0, -1);
    glVertex3f(rx, -ry, 0);
    glVertex3f(rx, ry, 0);
    glVertex3f(-rx, ry, 0);
    glVertex3f(-rx, -ry, 0);
    glEnd();
}

GLvoid glBar() {
    float constexpr bar_height = .1500;
    float constexpr bar_bottom_half_width = .2427;
    float constexpr bar_bottom_half_length = .3927;
    float constexpr bar_top_half_width = .1500;
    float constexpr bar_top_half_length = .2427;

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(-.7071, 0, .7071);
    glVertex3f(-bar_top_half_length,  bar_top_half_width, bar_height);
    glVertex3f(-bar_top_half_length, -bar_top_half_width, bar_height);
    glVertex3f(-bar_bottom_half_length, -bar_bottom_half_width, 0);
    glVertex3f(-bar_bottom_half_length,  bar_bottom_half_width, 0);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(.7071, 0, .7071);
    glVertex3f(bar_top_half_length, -bar_top_half_width, bar_height);
    glVertex3f(bar_top_half_length,  bar_top_half_width, bar_height);
    glVertex3f(bar_bottom_half_length,  bar_bottom_half_width, 0);
    glVertex3f(bar_bottom_half_length, -bar_bottom_half_width, 0);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    glVertex3f(-bar_top_half_length,  bar_top_half_width, bar_height);
    glVertex3f( bar_top_half_length,  bar_top_half_width, bar_height);
    glVertex3f( bar_top_half_length, -bar_top_half_width, bar_height);
    glVertex3f(-bar_top_half_length, -bar_top_half_width, bar_height);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, -.8506, .5258);
    glVertex3f(-bar_top_half_length, -bar_top_half_width, bar_height);
    glVertex3f( bar_top_half_length, -bar_top_half_width, bar_height);
    glVertex3f( bar_bottom_half_length, -bar_bottom_half_width, 0);
    glVertex3f(-bar_bottom_half_length, -bar_bottom_half_width, 0);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1);
    glVertex3f(-bar_bottom_half_length, -bar_bottom_half_width, 0);
    glVertex3f( bar_bottom_half_length, -bar_bottom_half_width, 0);
    glVertex3f( bar_bottom_half_length,  bar_bottom_half_width, 0);
    glVertex3f(-bar_bottom_half_length,  bar_bottom_half_width, 0);
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, .8506, .5258);
    glVertex3f(-bar_bottom_half_length,  bar_bottom_half_width, 0);
    glVertex3f( bar_bottom_half_length,  bar_bottom_half_width, 0);
    glVertex3f( bar_top_half_length,  bar_top_half_width, bar_height);
    glVertex3f(-bar_top_half_length,  bar_top_half_width, bar_height);
    glEnd();
}

GLvoid glEllipsoid(vector3f const & o, vector3f const & x, vector3f const & y, vector3f const & z, dword fcnt) {
    dword zcnt = fcnt / 2;
    for (dword i = 0; i < zcnt; ++i) {
        float cosphi0 = cos(M_PI / zcnt * i);
        float sinphi0 = sin(M_PI / zcnt * i);
        float cosphi1 = cos(M_PI / zcnt * (i + 1));
        float sinphi1 = sin(M_PI / zcnt * (i + 1));
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = 0; _j <= fcnt; ++_j) {
            dword j = _j % fcnt;
            float costheta = cos(M_PI * 2 / fcnt * j);
            float sintheta = sin(M_PI * 2 / fcnt * j);
            glNormal3fv((y * sinphi0 * costheta - x * sinphi0 * sintheta)
                  .cross(x * cosphi0 * costheta + y * cosphi0 * sintheta + z * sinphi0).normalize());
            glVertex3fv(o + x * sinphi0 * costheta + y * sinphi0 * sintheta - z * cosphi0);
            glNormal3fv((y * sinphi1 * costheta - x * sinphi1 * sintheta)
                  .cross(x * cosphi1 * costheta + y * cosphi1 * sintheta + z * sinphi1).normalize());
            glVertex3fv(o + x * sinphi1 * costheta + y * sinphi1 * sintheta - z * cosphi1);
        }
        glEnd();
    }
}

// k < 0.25
GLvoid glCushion(vector3f const & o, vector3f const & x, vector3f const & y, vector3f const & zn, vector3f const & zp, float k, dword fcnt) {
    dword zcnt_2 = fcnt / 4;
    dword fcnt_8 = fcnt / 8;
    dword fcnt_4 = fcnt_8 * 2;
    dword zcnt = zcnt_2 * 2;
    fcnt = fcnt_4 * 4;

    for (dword i = 0; i < zcnt_2; ++i) {
        float cosphi0 = cos(M_PI / zcnt * i);
        float sinphi0 = sin(M_PI / zcnt * i);
        float cosphi1 = cos(M_PI / zcnt * (i + 1));
        float sinphi1 = sin(M_PI / zcnt * (i + 1));
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = fcnt_8; _j <= fcnt_8 * 3; ++_j) {
                dword j = _j % fcnt;
                float costheta = cos(M_PI * 2 / fcnt * j);
                float cos2theta = cos(M_PI * 4 / fcnt * j);
                float sintheta = sin(M_PI * 2 / fcnt * j);
                float sin2theta = sin(M_PI * 4 / fcnt * j);
                glNormal3fv((x * sinphi0 * (- 2 * k * sin2theta * costheta - (1 + k + k * cos2theta) * sintheta)
                           + y * sinphi0 * (- 2 * k * sin2theta * sintheta + (1 + k + k * cos2theta) * costheta))
                      .cross(x * cosphi0 * costheta * (1 + k + k * cos2theta)
                           + y * cosphi0 * sintheta * (1 + k + k * cos2theta)
                           - zn * sinphi0).normalize());
                glVertex3fv(o + x * sinphi0 * costheta * (1 + k + k * cos2theta)
                              + y * sinphi0 * sintheta * (1 + k + k * cos2theta)
                              + zn * cosphi0);

                glNormal3fv((x * sinphi1 * (- 2 * k * sin2theta * costheta - (1 + k + k * cos2theta) * sintheta)
                           + y * sinphi1 * (- 2 * k * sin2theta * sintheta + (1 + k + k * cos2theta) * costheta))
                      .cross(x * cosphi1 * costheta * (1 + k + k * cos2theta)
                           + y * cosphi1 * sintheta * (1 + k + k * cos2theta)
                           - zn * sinphi1).normalize());
                glVertex3fv(o + x * sinphi1 * costheta * (1 + k + k * cos2theta)
                              + y * sinphi1 * sintheta * (1 + k + k * cos2theta)
                              + zn * cosphi1);
            }
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = fcnt_8 * 3; _j <= fcnt_8 * 5; ++_j) {
            dword j = _j % fcnt;
            float costheta = cos(M_PI * 2 / fcnt * j);
            float cos2theta = cos(M_PI * 4 / fcnt * j);
            float sintheta = sin(M_PI * 2 / fcnt * j);
            float sin2theta = sin(M_PI * 4 / fcnt * j);
            glNormal3fv((x * sinphi0 * (2 * k * sin2theta * costheta - (1 + k - k * cos2theta) * sintheta)
                       + y * sinphi0 * (2 * k * sin2theta * sintheta + (1 + k - k * cos2theta) * costheta))
                  .cross(x * cosphi0 * costheta * (1 + k - k * cos2theta)
                       + y * cosphi0 * sintheta * (1 + k - k * cos2theta)
                       - zn * sinphi0).normalize());
            glVertex3fv(o + x * sinphi0 * costheta * (1 + k - k * cos2theta)
                          + y * sinphi0 * sintheta * (1 + k - k * cos2theta)
                          + zn * cosphi0);

            glNormal3fv((x * sinphi1 * (2 * k * sin2theta * costheta - (1 + k - k * cos2theta) * sintheta)
                       + y * sinphi1 * (2 * k * sin2theta * sintheta + (1 + k - k * cos2theta) * costheta))
                  .cross(x * cosphi1 * costheta * (1 + k - k * cos2theta)
                       + y * cosphi1 * sintheta * (1 + k - k * cos2theta)
                       - zn * sinphi1).normalize());
            glVertex3fv(o + x * sinphi1 * costheta * (1 + k - k * cos2theta)
                          + y * sinphi1 * sintheta * (1 + k - k * cos2theta)
                          + zn * cosphi1);
        }
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = fcnt_8 * 5; _j <= fcnt_8 * 7; ++_j) {
                dword j = _j % fcnt;
                float costheta = cos(M_PI * 2 / fcnt * j);
                float cos2theta = cos(M_PI * 4 / fcnt * j);
                float sintheta = sin(M_PI * 2 / fcnt * j);
                float sin2theta = sin(M_PI * 4 / fcnt * j);
                glNormal3fv((x * sinphi0 * (- 2 * k * sin2theta * costheta - (1 + k + k * cos2theta) * sintheta)
                           + y * sinphi0 * (- 2 * k * sin2theta * sintheta + (1 + k + k * cos2theta) * costheta))
                      .cross(x * cosphi0 * costheta * (1 + k + k * cos2theta)
                           + y * cosphi0 * sintheta * (1 + k + k * cos2theta)
                           - zn * sinphi0).normalize());
                glVertex3fv(o + x * sinphi0 * costheta * (1 + k + k * cos2theta)
                              + y * sinphi0 * sintheta * (1 + k + k * cos2theta)
                              + zn * cosphi0);

                glNormal3fv((x * sinphi1 * (- 2 * k * sin2theta * costheta - (1 + k + k * cos2theta) * sintheta)
                           + y * sinphi1 * (- 2 * k * sin2theta * sintheta + (1 + k + k * cos2theta) * costheta))
                      .cross(x * cosphi1 * costheta * (1 + k + k * cos2theta)
                           + y * cosphi1 * sintheta * (1 + k + k * cos2theta)
                           - zn * sinphi1).normalize());
                glVertex3fv(o + x * sinphi1 * costheta * (1 + k + k * cos2theta)
                              + y * sinphi1 * sintheta * (1 + k + k * cos2theta)
                              + zn * cosphi1);
            }
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = fcnt_8 * 7; _j <= fcnt_8 * 9; ++_j) {
            dword j = _j % fcnt;
            float costheta = cos(M_PI * 2 / fcnt * j);
            float cos2theta = cos(M_PI * 4 / fcnt * j);
            float sintheta = sin(M_PI * 2 / fcnt * j);
            float sin2theta = sin(M_PI * 4 / fcnt * j);
            glNormal3fv((x * sinphi0 * (2 * k * sin2theta * costheta - (1 + k - k * cos2theta) * sintheta)
                       + y * sinphi0 * (2 * k * sin2theta * sintheta + (1 + k - k * cos2theta) * costheta))
                  .cross(x * cosphi0 * costheta * (1 + k - k * cos2theta)
                       + y * cosphi0 * sintheta * (1 + k - k * cos2theta)
                       - zn * sinphi0).normalize());
            glVertex3fv(o + x * sinphi0 * costheta * (1 + k - k * cos2theta)
                          + y * sinphi0 * sintheta * (1 + k - k * cos2theta)
                          + zn * cosphi0);

            glNormal3fv((x * sinphi1 * (2 * k * sin2theta * costheta - (1 + k - k * cos2theta) * sintheta)
                       + y * sinphi1 * (2 * k * sin2theta * sintheta + (1 + k - k * cos2theta) * costheta))
                  .cross(x * cosphi1 * costheta * (1 + k - k * cos2theta)
                       + y * cosphi1 * sintheta * (1 + k - k * cos2theta)
                       - zn * sinphi1).normalize());
            glVertex3fv(o + x * sinphi1 * costheta * (1 + k - k * cos2theta)
                          + y * sinphi1 * sintheta * (1 + k - k * cos2theta)
                          + zn * cosphi1);
        }
        glEnd();
    }

    for (dword i = zcnt_2; i < zcnt; ++i) {
        float cosphi0 = cos(M_PI / zcnt * i);
        float sinphi0 = sin(M_PI / zcnt * i);
        float cosphi1 = cos(M_PI / zcnt * (i + 1));
        float sinphi1 = sin(M_PI / zcnt * (i + 1));
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = fcnt_8; _j <= fcnt_8 * 3; ++_j) {
                dword j = _j % fcnt;
                float costheta = cos(M_PI * 2 / fcnt * j);
                float cos2theta = cos(M_PI * 4 / fcnt * j);
                float sintheta = sin(M_PI * 2 / fcnt * j);
                float sin2theta = sin(M_PI * 4 / fcnt * j);
                glNormal3fv((x * sinphi0 * (- 2 * k * sin2theta * costheta - (1 + k + k * cos2theta) * sintheta)
                           + y * sinphi0 * (- 2 * k * sin2theta * sintheta + (1 + k + k * cos2theta) * costheta))
                      .cross(x * cosphi0 * costheta * (1 + k + k * cos2theta)
                           + y * cosphi0 * sintheta * (1 + k + k * cos2theta)
                           + zp * sinphi0).normalize());
                glVertex3fv(o + x * sinphi0 * costheta * (1 + k + k * cos2theta)
                              + y * sinphi0 * sintheta * (1 + k + k * cos2theta)
                              - zp * cosphi0);

                glNormal3fv((x * sinphi1 * (- 2 * k * sin2theta * costheta - (1 + k + k * cos2theta) * sintheta)
                           + y * sinphi1 * (- 2 * k * sin2theta * sintheta + (1 + k + k * cos2theta) * costheta))
                      .cross(x * cosphi1 * costheta * (1 + k + k * cos2theta)
                           + y * cosphi1 * sintheta * (1 + k + k * cos2theta)
                           + zp * sinphi1).normalize());
                glVertex3fv(o + x * sinphi1 * costheta * (1 + k + k * cos2theta)
                              + y * sinphi1 * sintheta * (1 + k + k * cos2theta)
                              - zp * cosphi1);
            }
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = fcnt_8 * 3; _j <= fcnt_8 * 5; ++_j) {
            dword j = _j % fcnt;
            float costheta = cos(M_PI * 2 / fcnt * j);
            float cos2theta = cos(M_PI * 4 / fcnt * j);
            float sintheta = sin(M_PI * 2 / fcnt * j);
            float sin2theta = sin(M_PI * 4 / fcnt * j);
            glNormal3fv((x * sinphi0 * (2 * k * sin2theta * costheta - (1 + k - k * cos2theta) * sintheta)
                       + y * sinphi0 * (2 * k * sin2theta * sintheta + (1 + k - k * cos2theta) * costheta))
                  .cross(x * cosphi0 * costheta * (1 + k - k * cos2theta)
                       + y * cosphi0 * sintheta * (1 + k - k * cos2theta)
                       + zp * sinphi0).normalize());
            glVertex3fv(o + x * sinphi0 * costheta * (1 + k - k * cos2theta)
                          + y * sinphi0 * sintheta * (1 + k - k * cos2theta)
                          - zp * cosphi0);

            glNormal3fv((x * sinphi1 * (2 * k * sin2theta * costheta - (1 + k - k * cos2theta) * sintheta)
                       + y * sinphi1 * (2 * k * sin2theta * sintheta + (1 + k - k * cos2theta) * costheta))
                  .cross(x * cosphi1 * costheta * (1 + k - k * cos2theta)
                       + y * cosphi1 * sintheta * (1 + k - k * cos2theta)
                       + zp * sinphi1).normalize());
            glVertex3fv(o + x * sinphi1 * costheta * (1 + k - k * cos2theta)
                          + y * sinphi1 * sintheta * (1 + k - k * cos2theta)
                          - zp * cosphi1);
        }
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = fcnt_8 * 5; _j <= fcnt_8 * 7; ++_j) {
                dword j = _j % fcnt;
                float costheta = cos(M_PI * 2 / fcnt * j);
                float cos2theta = cos(M_PI * 4 / fcnt * j);
                float sintheta = sin(M_PI * 2 / fcnt * j);
                float sin2theta = sin(M_PI * 4 / fcnt * j);
                glNormal3fv((x * sinphi0 * (- 2 * k * sin2theta * costheta - (1 + k + k * cos2theta) * sintheta)
                           + y * sinphi0 * (- 2 * k * sin2theta * sintheta + (1 + k + k * cos2theta) * costheta))
                      .cross(x * cosphi0 * costheta * (1 + k + k * cos2theta)
                           + y * cosphi0 * sintheta * (1 + k + k * cos2theta)
                           + zp * sinphi0).normalize());
                glVertex3fv(o + x * sinphi0 * costheta * (1 + k + k * cos2theta)
                              + y * sinphi0 * sintheta * (1 + k + k * cos2theta)
                              - zp * cosphi0);

                glNormal3fv((x * sinphi1 * (- 2 * k * sin2theta * costheta - (1 + k + k * cos2theta) * sintheta)
                           + y * sinphi1 * (- 2 * k * sin2theta * sintheta + (1 + k + k * cos2theta) * costheta))
                      .cross(x * cosphi1 * costheta * (1 + k + k * cos2theta)
                           + y * cosphi1 * sintheta * (1 + k + k * cos2theta)
                           + zp * sinphi1).normalize());
                glVertex3fv(o + x * sinphi1 * costheta * (1 + k + k * cos2theta)
                              + y * sinphi1 * sintheta * (1 + k + k * cos2theta)
                              - zp * cosphi1);
            }
        glEnd();
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = fcnt_8 * 7; _j <= fcnt_8 * 9; ++_j) {
            dword j = _j % fcnt;
            float costheta = cos(M_PI * 2 / fcnt * j);
            float cos2theta = cos(M_PI * 4 / fcnt * j);
            float sintheta = sin(M_PI * 2 / fcnt * j);
            float sin2theta = sin(M_PI * 4 / fcnt * j);
            glNormal3fv((x * sinphi0 * (2 * k * sin2theta * costheta - (1 + k - k * cos2theta) * sintheta)
                       + y * sinphi0 * (2 * k * sin2theta * sintheta + (1 + k - k * cos2theta) * costheta))
                  .cross(x * cosphi0 * costheta * (1 + k - k * cos2theta)
                       + y * cosphi0 * sintheta * (1 + k - k * cos2theta)
                       + zp * sinphi0).normalize());
            glVertex3fv(o + x * sinphi0 * costheta * (1 + k - k * cos2theta)
                          + y * sinphi0 * sintheta * (1 + k - k * cos2theta)
                          - zp * cosphi0);

            glNormal3fv((x * sinphi1 * (2 * k * sin2theta * costheta - (1 + k - k * cos2theta) * sintheta)
                       + y * sinphi1 * (2 * k * sin2theta * sintheta + (1 + k - k * cos2theta) * costheta))
                  .cross(x * cosphi1 * costheta * (1 + k - k * cos2theta)
                       + y * cosphi1 * sintheta * (1 + k - k * cos2theta)
                       + zp * sinphi1).normalize());
            glVertex3fv(o + x * sinphi1 * costheta * (1 + k - k * cos2theta)
                          + y * sinphi1 * sintheta * (1 + k - k * cos2theta)
                          - zp * cosphi1);
        }
        glEnd();
    }
}

GLvoid glRectCabochon(vector3f const & o, vector3f const & x, vector3f const & y, vector3f const & z, float k, dword fcnt) {
    dword zcnt = fcnt / 2;
    for (dword i = 0; i < zcnt; ++i) {
        float cosphi0 = cos(M_PI / zcnt * i);
        float sinphi0 = sin(M_PI / zcnt * i);
        float cosphi1 = cos(M_PI / zcnt * (i + 1));
        float sinphi1 = sin(M_PI / zcnt * (i + 1));
        glBegin(GL_TRIANGLE_STRIP);
        for (dword _j = 0; _j <= fcnt; ++_j) {
            dword j = _j % fcnt;
            float costheta = cos(M_PI * 2 / fcnt * j);
            float sintheta = sin(M_PI * 2 / fcnt * j);
            float cos4theta = cos(M_PI * 8 / fcnt * j);
            float sin4theta = sin(M_PI * 8 / fcnt * j);
            glNormal3fv((x * sinphi0 * (-sintheta * (1 + k - k * cos4theta) + 4 * k * costheta * sin4theta)
                       + y * sinphi0 * ( costheta * (1 + k - k * cos4theta) + 4 * k * sintheta * sin4theta))
                  .cross(x * cosphi0 * costheta * (1 + k - k * cos4theta)
                       + y * cosphi0 * sintheta * (1 + k - k * cos4theta)
                       + z * sinphi0).normalize());
            glVertex3fv(o + x * sinphi0 * costheta * (1 + k - k * cos4theta)
                          + y * sinphi0 * sintheta * (1 + k - k * cos4theta)
                          - z * cosphi0);

            glNormal3fv((x * sinphi1 * (-sintheta * (1 + k - k * cos4theta) + 4 * k * costheta * sin4theta)
                       + y * sinphi1 * ( costheta * (1 + k - k * cos4theta) + 4 * k * sintheta * sin4theta))
                  .cross(x * cosphi1 * costheta * (1 + k - k * cos4theta)
                       + y * cosphi1 * sintheta * (1 + k - k * cos4theta)
                       + z * sinphi1).normalize());
            glVertex3fv(o + x * sinphi1 * costheta * (1 + k - k * cos4theta)
                          + y * sinphi1 * sintheta * (1 + k - k * cos4theta)
                          - z * cosphi1);
        }
        glEnd();
    }
}

GLvoid glPointCut(float rz, float rxy) {
    glBegin(GL_TRIANGLES);
    for (dword i = 0; i < 4; ++i) {
        vector3f l0(cos(M_PI / 2 * (i - .5)) * rxy, sin(M_PI / 2 * (i - .5)) * rxy, rz);
        vector3f l1(cos(M_PI / 2 * (i + .5)) * rxy, sin(M_PI / 2 * (i + .5)) * rxy, rz);
        glNormal3fv(l1.cross(l0).normalize());
        glVertex3f(0, 0, 0);
        glVertex3fv(l0);
        glVertex3fv(l1);
        vector3f top(0, 0, rz * 2);
        glNormal3fv((l0 - top).cross(l1 - top).normalize());
        glVertex3fv(top);
        glVertex3fv(l1);
        glVertex3fv(l0);
    }
    glEnd();
}

GLvoid glTableCut(float rz, float rxy) {
    glBegin(GL_TRIANGLES);
    for (dword i = 0; i < 4; ++i) {
        vector3f l0(cos(M_PI / 2 * (i - .5)) * rxy, sin(M_PI / 2 * (i - .5)) * rxy, rz);
        vector3f l1(cos(M_PI / 2 * (i + .5)) * rxy, sin(M_PI / 2 * (i + .5)) * rxy, rz);
        glNormal3fv(l1.cross(l0).normalize());
        glVertex3f(0, 0, 0);
        glVertex3fv(l0);
        glVertex3fv(l1);
    }
    glEnd();
    vector3f top(0, 0, rz * 2);
    glBegin(GL_QUADS);
    for (dword i = 0; i < 4; ++i) {
        vector3f l0(cos(M_PI / 2 * (i - .5)) * rxy, sin(M_PI / 2 * (i - .5)) * rxy, rz);
        vector3f l1(cos(M_PI / 2 * (i + .5)) * rxy, sin(M_PI / 2 * (i + .5)) * rxy, rz);
        glNormal3fv((l0 - top).cross(l1 - top).normalize());
        glVertex3fv(top + (l0 - top) * .5);
        glVertex3fv(top + (l1 - top) * .5);
        glVertex3fv(l1);
        glVertex3fv(l0);
    }
    glNormal3f(0, 0, 1);
    for (int32 i = 3; i >= 0; --i) {
        vector3f l0(cos(M_PI / 2 * (i - .5)) * rxy, sin(M_PI / 2 * (i - .5)) * rxy, rz);
        glVertex3fv(top + (l0 - top) * .5);
    }
    glEnd();
}

GLvoid glDoor(vector3f b, vector3f x, vector3f y, vector3f z) {
    vector3f wb = b + z * 5 / 8, wx = x / 2, wy = y, wz = z / 4;
    glBegin(GL_QUADS);
    glNormal3fv(x.cross(y).normalize());
    glVertex3fv(b + ( x - y) / 2 + z);
    glVertex3fv(b + (-x - y) / 2 + z);
    glVertex3fv(b + (-x + y) / 2 + z);
    glVertex3fv(b + ( x + y) / 2 + z);
    glVertex3fv(wb + ( wx - wy) / 2);
    glVertex3fv(wb + (-wx - wy) / 2);
    glVertex3fv(wb + (-wx + wy) / 2);
    glVertex3fv(wb + ( wx + wy) / 2);
    glNormal3fv(y.cross(z).normalize());
    glVertex3fv(b + ( x - y) / 2 + z);
    glVertex3fv(b + ( x + y) / 2 + z);
    glVertex3fv(b + ( x + y) / 2);
    glVertex3fv(b + ( x - y) / 2);
    glVertex3fv(wb + (-wx - wy) / 2 + wz);
    glVertex3fv(wb + (-wx + wy) / 2 + wz);
    glVertex3fv(wb + (-wx + wy) / 2);
    glVertex3fv(wb + (-wx - wy) / 2);
    glNormal3fv(z.cross(y).normalize());
    glVertex3fv(b + (-x + y) / 2 + z);
    glVertex3fv(b + (-x - y) / 2 + z);
    glVertex3fv(b + (-x - y) / 2);
    glVertex3fv(b + (-x + y) / 2);
    glVertex3fv(wb + ( wx + wy) / 2 + wz);
    glVertex3fv(wb + ( wx - wy) / 2 + wz);
    glVertex3fv(wb + ( wx - wy) / 2);
    glVertex3fv(wb + ( wx + wy) / 2);
    glNormal3fv(y.cross(x).normalize());
    glVertex3fv(b + ( x - y) / 2);
    glVertex3fv(b + ( x + y) / 2);
    glVertex3fv(b + (-x + y) / 2);
    glVertex3fv(b + (-x - y) / 2);
    glVertex3fv(wb + ( wx - wy) / 2 + wz);
    glVertex3fv(wb + ( wx + wy) / 2 + wz);
    glVertex3fv(wb + (-wx + wy) / 2 + wz);
    glVertex3fv(wb + (-wx - wy) / 2 + wz);
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3fv(z.cross(x).normalize());
    glVertex3fv(wb + ( wx + wy) / 2 + wz);
    glVertex3fv( b + (  x +  y) / 2 +  z);
    glVertex3fv(wb + (-wx + wy) / 2 + wz);
    glVertex3fv( b + (- x +  y) / 2 +  z);
    glVertex3fv(wb + (-wx + wy) / 2);
    glVertex3fv( b + (- x +  y) / 2);
    glVertex3fv(wb + ( wx + wy) / 2);
    glVertex3fv( b + (  x +  y) / 2);
    glVertex3fv(wb + ( wx + wy) / 2 + wz);
    glVertex3fv( b + (  x +  y) / 2 +  z);
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3fv(x.cross(z).normalize());
    glVertex3fv(wb + ( wx - wy) / 2 + wz);
    glVertex3fv( b + (  x -  y) / 2 +  z);
    glVertex3fv(wb + ( wx - wy) / 2);
    glVertex3fv( b + (  x -  y) / 2);
    glVertex3fv(wb + (-wx - wy) / 2);
    glVertex3fv( b + (- x -  y) / 2);
    glVertex3fv(wb + (-wx - wy) / 2 + wz);
    glVertex3fv( b + (- x -  y) / 2 +  z);
    glVertex3fv(wb + ( wx - wy) / 2 + wz);
    glVertex3fv( b + (  x -  y) / 2 +  z);
    glEnd();
//    float handlesize = (y / 10).mod();
//    glEllipsoid(b + x / 4 + z / 2, x.normalize() * handlesize, y.normalize() * (.25 + handlesize), z.normalize() * handlesize, 8);
}

GLvoid glFloodGate(vector3f b, vector3f x, vector3f y, vector3f z) {
    vector3f wb = b + z / 8, wx = x / 2, wy = y, wz = z * 3 / 8;
    vector3f gb = wb, gx = wx, gy = wy / 2, gz = wz;
    glBegin(GL_QUADS);
    glNormal3fv(x.cross(y).normalize());
    glVertex3fv(b + ( x - y) / 2 + z);
    glVertex3fv(b + (-x - y) / 2 + z);
    glVertex3fv(b + (-x + y) / 2 + z);
    glVertex3fv(b + ( x + y) / 2 + z);
    glVertex3fv(wb + ( wx - wy) / 2);
    glVertex3fv(wb + (-wx - wy) / 2);
    glVertex3fv(wb + (-wx + wy) / 2);
    glVertex3fv(wb + ( wx + wy) / 2);
    glNormal3fv(y.cross(z).normalize());
    glVertex3fv(b + ( x - y) / 2 + z);
    glVertex3fv(b + ( x + y) / 2 + z);
    glVertex3fv(b + ( x + y) / 2);
    glVertex3fv(b + ( x - y) / 2);
    glVertex3fv(wb + (-wx - wy) / 2 + wz);
    glVertex3fv(wb + (-wx + wy) / 2 + wz);
    glVertex3fv(wb + (-wx + wy) / 2);
    glVertex3fv(wb + (-wx - wy) / 2);
    glNormal3fv(z.cross(x).normalize());
    glVertex3fv(gb + ( gx + gy) / 2 + gz);
    glVertex3fv(gb + (-gx + gy) / 2 + gz);
    glVertex3fv(gb + (-gx + gy) / 2);
    glVertex3fv(gb + ( gx + gy) / 2);
    glNormal3fv(z.cross(y).normalize());
    glVertex3fv(b + (-x + y) / 2 + z);
    glVertex3fv(b + (-x - y) / 2 + z);
    glVertex3fv(b + (-x - y) / 2);
    glVertex3fv(b + (-x + y) / 2);
    glVertex3fv(wb + ( wx + wy) / 2 + wz);
    glVertex3fv(wb + ( wx - wy) / 2 + wz);
    glVertex3fv(wb + ( wx - wy) / 2);
    glVertex3fv(wb + ( wx + wy) / 2);
    glNormal3fv(x.cross(z).normalize());
    glVertex3fv(gb + (-gx - gy) / 2 + gz);
    glVertex3fv(gb + ( gx - gy) / 2 + gz);
    glVertex3fv(gb + ( gx - gy) / 2);
    glVertex3fv(gb + (-gx - gy) / 2);
    glNormal3fv(y.cross(x).normalize());
    glVertex3fv(b + ( x - y) / 2);
    glVertex3fv(b + ( x + y) / 2);
    glVertex3fv(b + (-x + y) / 2);
    glVertex3fv(b + (-x - y) / 2);
    glVertex3fv(wb + ( wx - wy) / 2 + wz);
    glVertex3fv(wb + ( wx + wy) / 2 + wz);
    glVertex3fv(wb + (-wx + wy) / 2 + wz);
    glVertex3fv(wb + (-wx - wy) / 2 + wz);
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3fv(z.cross(x).normalize());
    glVertex3fv(wb + ( wx + wy) / 2 + wz);
    glVertex3fv( b + (  x +  y) / 2 +  z);
    glVertex3fv(wb + (-wx + wy) / 2 + wz);
    glVertex3fv( b + (- x +  y) / 2 +  z);
    glVertex3fv(wb + (-wx + wy) / 2);
    glVertex3fv( b + (- x +  y) / 2);
    glVertex3fv(wb + ( wx + wy) / 2);
    glVertex3fv( b + (  x +  y) / 2);
    glVertex3fv(wb + ( wx + wy) / 2 + wz);
    glVertex3fv( b + (  x +  y) / 2 +  z);
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    glNormal3fv(x.cross(z).normalize());
    glVertex3fv(wb + ( wx - wy) / 2 + wz);
    glVertex3fv( b + (  x -  y) / 2 +  z);
    glVertex3fv(wb + ( wx - wy) / 2);
    glVertex3fv( b + (  x -  y) / 2);
    glVertex3fv(wb + (-wx - wy) / 2);
    glVertex3fv( b + (- x -  y) / 2);
    glVertex3fv(wb + (-wx - wy) / 2 + wz);
    glVertex3fv( b + (- x -  y) / 2 +  z);
    glVertex3fv(wb + ( wx - wy) / 2 + wz);
    glVertex3fv( b + (  x -  y) / 2 +  z);
    glEnd();
}

GLvoid glSingleCut(float hz, float rxy) {
    glBegin(GL_TRIANGLES);
    for (dword i = 0; i < 8; ++i) {
        vector3f r0(cos(M_PI / 4 * i) * rxy, sin(M_PI / 4 * i) * rxy, hz);
        vector3f r1(cos(M_PI / 4 * (i + 1)) * rxy, sin(M_PI / 4 * (i + 1)) * rxy, hz);
        vector3f n = r1.cross(r0).normalize();
        glNormal3fv(n);
        glVertex3f(0, 0, 0);
        glVertex3fv(r0);
        glVertex3fv(r1);
    }
    glEnd();
    glBegin(GL_QUADS);
    vector3f top(0, 0, hz * 2);
    for (dword i = 0; i < 8; ++i) {
        vector3f r0(cos(M_PI / 4 * i) * rxy, sin(M_PI / 4 * i) * rxy, -hz);
        vector3f r1(cos(M_PI / 4 * (i + 1)) * rxy, sin(M_PI / 4 * (i + 1)) * rxy, -hz);
        vector3f n = r0.cross(r1).normalize();
        glNormal3fv(n);
        glVertex3fv(top + r1);
        glVertex3fv(top + r0);
        glVertex3fv(top + r0 * .5);
        glVertex3fv(top + r1 * .5);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    for (int32 i = 7; i >= 0; --i) {
        vector3f r(cos(M_PI / 4 * i) * rxy * .5, sin(M_PI / 4 * i) * rxy * .5, hz * 1.5);
        glVertex3fv(r);
    }
    glEnd();
}

GLvoid glRoseCut(float hn, float r1) {
    float r0 = sin(M_PI / 12) / sin(M_PI / 6);
    float h1 = sqrt((4 * sin(M_PI / 12) * sin(M_PI / 12) - (1 - r0) * (1 - r0)) / (1- r0 * r0));
    float h0 = sqrt(1 - r0 * r0) * h1;
    r0 *= r1;
    h1 = h1 * r1 + hn;
    h0 = h0 * r1 + hn;
    vector3f top(0, 0, h1);
    glBegin(GL_TRIANGLES);
    for (dword i = 0; i < 6; ++i) {
        float a00 = M_PI / 3 * i,
              a01 = M_PI / 3 * (i + 1),
              a10 = M_PI / 6 * (i * 2),
              a11 = M_PI / 6 * (i * 2 + 1),
              a12 = M_PI / 6 * (i * 2 + 2);
        float cosa00 = cos(a00), sina00 = sin(a00),
              cosa01 = cos(a01), sina01 = sin(a01),
              cosa10 = cos(a10), sina10 = sin(a10),
              cosa11 = cos(a11), sina11 = sin(a11),
              cosa12 = cos(a12), sina12 = sin(a12);
        vector3f l00(cosa00 * r0, sina00 * r0, h0),
                 l01(cosa01 * r0, sina01 * r0, h0),
                 l10(cosa10 * r1, sina10 * r1, hn),
                 l11(cosa11 * r1, sina11 * r1, hn),
                 l12(cosa12 * r1, sina12 * r1, hn);
        glNormal3fv((l00 - top).cross(l01 - top).normalize());
        glVertex3fv(top);
        glVertex3fv(l01);
        glVertex3fv(l00);
        glNormal3fv((l10 - l00).cross(l11 - l00).normalize());
        glVertex3fv(l00);
        glVertex3fv(l11);
        glVertex3fv(l10);
        glNormal3fv((l01 - l11).cross(l00 - l11).normalize());
        glVertex3fv(l11);
        glVertex3fv(l00);
        glVertex3fv(l01);
        glNormal3fv((l11 - l01).cross(l12 - l01).normalize());
        glVertex3fv(l01);
        glVertex3fv(l12);
        glVertex3fv(l11);
    }
    for (dword i = 0; i < 12; ++i) {
        float a10 = M_PI / 6 * i,
              a11 = M_PI / 6 * (i + 1);
        float cosa10 = cos(a10), sina10 = sin(a10),
              cosa11 = cos(a11), sina11 = sin(a11);
        vector3f l10(cosa10 * r1, sina10 * r1, hn),
                 l11(cosa11 * r1, sina11 * r1, hn);
        glNormal3fv(l11.cross(l10).normalize());
        glVertex3f(0, 0, 0);
        glVertex3fv(l10);
        glVertex3fv(l11);
    }
    glEnd();
}

//GLvoid glModel() {
//    float constexpr ratio = (sqrt(5) + 1) / 2;
//    float constexpr r0 = .1, r1 = r0 * ratio, r2 = r1 * ratio;
//    static float constexpr size[3] = {r2 * 2, r1 * 2, r1 * 2};

//    float constexpr s2 = 1, s1 = s2 / ratio, s0 = s1 / ratio;
//    static float constexpr size[3] = {s2, s2, s0};

//#if MODEL_OUTPUT
//    if (model_first) {
//        model.open("models/bed.mdl", ios_base::out | ios_base::trunc | ios_base::binary);
//        model.write((char *) size, sizeof(size));
//    }
//#endif

//    glBed({-s2 / 2, 0, s0 / 2} ,vector3f::ey * s2, vector3f::ez * s0, vector3f::ex * s2);

//#if MODEL_OUTPUT
//    if (began) {
//        glEndOut();
//        cout << "begin without an end!" << endl;
//    }
//    if (model_first) {
//        model_first = false;
//        model.close();
//    }
//#endif
//}

#endif

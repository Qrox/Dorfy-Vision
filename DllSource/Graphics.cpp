#ifdef BUILD_DLL

#include <ctime>
#include <cmath>
#include <sstream>

#include "gl/glew.h"
#include "Graphics.h"
#include "MainWindow.h"
#include "DwarfFortress.h"
#include "DwarfItem.h"
#include "DwarfMaterial.h"
#include "DwarfVegetation.h"
#include "DwarfCreature.h"
#include "Utils.h"
#include "Model.h"
#include "RenderRange.h"
#include "Vector3f.h"

#include <gl/glu.h>
#include <gl/glext.h>
// fix eclipse weird bug
//GLAPI void APIENTRY gluLookAt(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ);

using namespace DwarfFortress;

static float constexpr
    HUD_TOP = 5.85,
    HUD_BOTTOM = 1.0,
    HUD_LEFT = 0.5,
    HUD_RIGHT = 3.5;

static int constexpr
    HUD_WIDTH = 400,
    HUD_HEIGHT = ((int) (HUD_WIDTH / (HUD_RIGHT - HUD_LEFT) * (HUD_TOP - HUD_BOTTOM)));

HDC hdc = 0, hud = 0, hdc_text = 0;
HGLRC hrc = 0;
HFONT hud_font = 0, text_font = 0;
HBITMAP hud_bitmap = 0, text_bitmap = 0;
void * painted_hud = 0, * painted_text = 0;
GLint texture_max_size, text_bitmap_width, text_bitmap_height, text_font_size;

shader shader_vert, shader_frag;
shaderprogram shader_prog;
GLint shader_fog, shader_start, shader_k;

int glInit(HWND window) {
    cout << "initializing opengl ..." << endl;

    hdc = GetDC(window);
    if (hdc == NULL) {
        printError("failed to create a GL device context!");
        return 0;
    }
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
    if (pixel_format == 0) {
        printError("cannot find a suitable pixel format");
        return 0;
    }
    if (!SetPixelFormat(hdc, pixel_format, &pfd)) {
        printError("failed to set pixel format!");
        return 0;
    }
    hrc = wglCreateContext(hdc);
    if (hrc == NULL) {
        printError("failed to create a GL rendering context!");
        return 0;
    }
    if (!wglMakeCurrent(hdc, hrc)) {
        printError("failed to activate the GL rendering context!");
        return 0;
    }

    cout << "gl version: " << glGetString(GL_VERSION) << endl;
    GLenum res = glewInit();
    if (res != GLEW_OK) {
        cout << "glew initialization failed, error code " << res << endl;
        return 0;
    }
    if (!GLEW_VERSION_2_0) {
        cout << "opengl 2.0 not supported! fog fragment shader will not work" << endl;
    } else {
        do {
            shader_vert = shader(GL_VERTEX_SHADER, "shaders/shader.vs");
            if (!shader_vert.compiled()) break;
            shader_frag = shader(GL_FRAGMENT_SHADER, "shaders/shader.fs");
            if (!shader_frag.compiled()) break;
            shader_prog.create();
            shader_prog.attach(shader_vert);
            shader_prog.attach(shader_frag);
            if (!shader_prog.link()) {
                shader_prog.destroy();
                break;
            }
            shader_fog = shader_prog.getUniformLocation("fog");
            shader_start = shader_prog.getUniformLocation("start");
            shader_k = shader_prog.getUniformLocation("k");
        } while (false);
    }

    RECT rect;
    if (GetClientRect(dllWindow, &rect)) {
        glResize(rect.right - rect.left, rect.bottom - rect.top);
    }

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glEnable(GL_RESCALE_NORMAL); // prevents that changing the scale also changes brightness (the scaling matrix also scales the normals)
    glShadeModel(GL_SMOOTH);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0f);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

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
    font.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;
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

    // initialize hud
    hud = CreateCompatibleDC(NULL);
    SetBkMode(hud, TRANSPARENT);
    font.lfHeight = 40;
    hud_font = CreateFontIndirect(&font);
    DeleteObject(SelectObject(hud, hud_font));
    bminfo.bmiHeader.biWidth = HUD_WIDTH;
    bminfo.bmiHeader.biHeight = -HUD_HEIGHT;
    hud_bitmap = CreateDIBSection(0, &bminfo, DIB_RGB_COLORS, &painted_hud, NULL, 0);
    DeleteObject(SelectObject(hud, hud_bitmap));
    hudRepaint();

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

GLvoid glCursor() {
    static Model model("models/misc/cursor.mdl");
    model.glRender();
}

GLvoid glCube() {
    static float constexpr glCube_vertices[8][3] = {
        -.5, -.5,  0, //0
        -.5,  .5,  0, //1
         .5,  .5,  0, //2
         .5, -.5,  0, //3
        -.5, -.5,  1, //4
        -.5,  .5,  1, //5
         .5,  .5,  1, //6
         .5, -.5,  1, //7
    };
    static float constexpr glCube_normals[6][3] = {
         0,  0, -1,
         0, -1,  0,
        -1,  0,  0,
         0,  1,  0,
         1,  0,  0,
         0,  0,  1,
    };
    static byte const glCube_indices[6][5] = {
        0, 3, 2, 1, 0,
        3, 0, 4, 7, 1,
        0, 1, 5, 4, 2,
        1, 2, 6, 5, 3,
        2, 3, 7, 6, 4,
        5, 6, 7, 4, 5,
    };

    glBegin(GL_QUADS);
    for (int i = 0; i < 6; i++) {
        glNormal3fv(glCube_normals[glCube_indices[i][4]]);
        for (int j = 0; j < 4; j++) {
            glVertex3fv(glCube_vertices[glCube_indices[i][j]]);
        }
    }
    glEnd();
}

GLvoid glSphere() {
    dword constexpr vertex_cnt = 8;
    dword constexpr height_cnt = 5;
    static float constexpr shape[vertex_cnt][2] = {
        cos(M_PI * 2 / vertex_cnt * 0), sin(M_PI * 2 / vertex_cnt * 0),
        cos(M_PI * 2 / vertex_cnt * 1), sin(M_PI * 2 / vertex_cnt * 1),
        cos(M_PI * 2 / vertex_cnt * 2), sin(M_PI * 2 / vertex_cnt * 2),
        cos(M_PI * 2 / vertex_cnt * 3), sin(M_PI * 2 / vertex_cnt * 3),
        cos(M_PI * 2 / vertex_cnt * 4), sin(M_PI * 2 / vertex_cnt * 4),
        cos(M_PI * 2 / vertex_cnt * 5), sin(M_PI * 2 / vertex_cnt * 5),
        cos(M_PI * 2 / vertex_cnt * 6), sin(M_PI * 2 / vertex_cnt * 6),
        cos(M_PI * 2 / vertex_cnt * 7), sin(M_PI * 2 / vertex_cnt * 7),
    };

    for (dword i = 0; i < height_cnt - 1; ++i) {
        float z0 = shape[i    ][0], hz0 = z0 * .5 + .5, r0 = shape[i    ][1];
        float z1 = shape[i + 1][0], hz1 = z1 * .5 + .5, r1 = shape[i + 1][1];

        glBegin(GL_TRIANGLE_STRIP);
        for (dword j = 0; j < vertex_cnt; ++j) {
            float x0 = r0 * shape[j][0], y0 = r0 * shape[j][1];
            float hx0 = x0 * .5, hy0 = y0 * .5;
            float x1 = r1 * shape[j][0], y1 = r1 * shape[j][1];
            float hx1 = x1 * .5, hy1 = y1 * .5;
            glNormal3f(x1, y1, z1);
            glVertex3f(hx1, hy1, hz1);
            glNormal3f(x0, y0, z0);
            glVertex3f(hx0, hy0, hz0);
        }
        float x0 = r0 * shape[0][0], y0 = r0 * shape[0][1];
        float hx0 = x0 * .5, hy0 = y0 * .5;
        float x1 = r1 * shape[0][0], y1 = r1 * shape[0][1];
        float hx1 = x1 * .5, hy1 = y1 * .5;
        glNormal3f(x1, y1, z1);
        glVertex3f(hx1, hy1, hz1);
        glNormal3f(x0, y0, z0);
        glVertex3f(hx0, hy0, hz0);
        glEnd();
    }
}

GLvoid glCylinder() {
    dword constexpr vertex_cnt = 8;
    static float constexpr shape[vertex_cnt][2] = {
        cos(M_PI * 2 / vertex_cnt * 0), sin(M_PI * 2 / vertex_cnt * 0),
        cos(M_PI * 2 / vertex_cnt * 1), sin(M_PI * 2 / vertex_cnt * 1),
        cos(M_PI * 2 / vertex_cnt * 2), sin(M_PI * 2 / vertex_cnt * 2),
        cos(M_PI * 2 / vertex_cnt * 3), sin(M_PI * 2 / vertex_cnt * 3),
        cos(M_PI * 2 / vertex_cnt * 4), sin(M_PI * 2 / vertex_cnt * 4),
        cos(M_PI * 2 / vertex_cnt * 5), sin(M_PI * 2 / vertex_cnt * 5),
        cos(M_PI * 2 / vertex_cnt * 6), sin(M_PI * 2 / vertex_cnt * 6),
        cos(M_PI * 2 / vertex_cnt * 7), sin(M_PI * 2 / vertex_cnt * 7),
    };

    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, -1);
    for (dword i = 0; i < vertex_cnt; ++i) {
        glVertex3f(shape[i][0] * .5, shape[i][1] * .5, 0);
    }
    glEnd();
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0, 0, 1);
    for (int32 i = vertex_cnt - 1; i >= 0; --i) {
        glVertex3f(shape[i][0] * .5, shape[i][1] * .5, 1);
    }
    glEnd();
    glBegin(GL_TRIANGLE_STRIP);
    for (dword i = 0; i < vertex_cnt; ++i) {
        float x = shape[i][0], y = shape[i][1];
        float hx = x * .5, hy = y * .5;
        glNormal3f(x, y, 0);
        glVertex3f(hx, hy, 0);
        glVertex3f(hx, hy, 1);
    }
    float x = shape[0][0], y = shape[0][1];
    float hx = x * .5, hy = y * .5;
    glNormal3f(x, y, 0);
    glVertex3f(hx, hy, 0);
    glVertex3f(hx, hy, 1);
    glEnd();
}

GLvoid glRampUp(int x, int y, int z) {
    using namespace DwarfFortress::BodyType;
    static float constexpr glRampUp_indices[][2] = {
         .5,   0,
         .5, -.5,
          0, -.5,
        -.5, -.5,
        -.5,   0,
        -.5,  .5,
          0,  .5,
         .5,  .5
    };
    static float constexpr glRampUp_normal_side[][3] = {
         1,  0,  0,
         0, -1,  0,
        -1,  0,  0,
         0,  1,  0,
    };

    TileDefinition me = df.map.tileDefinition(x, y, z);
    TileDefinition conn[8];
    conn[0] = df.map.tileDefinition(x + 1, y    , z);
    conn[1] = df.map.tileDefinition(x + 1, y - 1, z);
    conn[2] = df.map.tileDefinition(x    , y - 1, z);
    conn[3] = df.map.tileDefinition(x - 1, y - 1, z);
    conn[4] = df.map.tileDefinition(x - 1, y    , z);
    conn[5] = df.map.tileDefinition(x - 1, y + 1, z);
    conn[6] = df.map.tileDefinition(x    , y + 1, z);
    conn[7] = df.map.tileDefinition(x + 1, y + 1, z);
    float ramp[8];
    float center = .5;
    for (int i = 0; i < 8; i++) {
        TileDefinition prev = conn[(i - 1) & 7],
                       curr = conn[ i         ],
                       next = conn[(i + 1) & 7];
        if (me.treetype) { // is tree slope
            if ((i & 1) ? (prev.treetype && prev.isblocked) ||
                          (curr.treetype && curr.isblocked) ||
                          (next.treetype && next.isblocked)
                        : (curr.treetype && curr.isblocked)) {
                ramp[i] = 1;
            } else if ((i & 1) ? prev.treetype && prev.isupslope &&
                                 curr.treetype && curr.isupslope &&
                                 next.treetype && next.isupslope
                               : curr.treetype && curr.isupslope) {
                ramp[i] = .5;
            } else {
                ramp[i] = 0;
            }
        } else {
            if ((i & 1) ? prev.isblocked || curr.isblocked || next.isblocked
                        : curr.isblocked) {
                ramp[i] = 1;
            } else if ((i & 1) ? prev.isupslope && curr.isupslope && next.isupslope
                               : curr.isupslope) {
                ramp[i] = .5;
            } else {
                ramp[i] = 0;
            }
        }
    }
    for (int i = 0; i < 8; i += 2) {
        float prev = ramp[(i - 1) & 7],
              next = ramp[(i + 1) & 7];
        if ((prev == 0 && next == 1) || (prev == 1 && next == 0)) {
            ramp[i] = .5;
        }
    }
    glBegin(GL_QUADS);
    glNormal3f(0, 0, -1);
    for (int i = 7; i > 0; i -= 2) {
        glVertex3f(glRampUp_indices[i][0], glRampUp_indices[i][1], 0);
    }
    glEnd();
    for (int i = 0; i < 8; i++) {
        int j = (i + 1) & 7;
        glBegin(GL_TRIANGLES);
        glNormal3fv(vector3f(glRampUp_indices[j][0], glRampUp_indices[j][1], ramp[j] - center)
             .cross(vector3f(glRampUp_indices[i][0], glRampUp_indices[i][1], ramp[i] - center)).normalize());
        glVertex3f(0, 0, center);
        glVertex3f(glRampUp_indices[i][0], glRampUp_indices[i][1], ramp[i]);
        glVertex3f(glRampUp_indices[j][0], glRampUp_indices[j][1], ramp[j]);
        glEnd();
        glBegin(GL_QUADS);
        glNormal3fv(glRampUp_normal_side[j / 2]);
        glVertex3f(glRampUp_indices[i][0], glRampUp_indices[i][1], ramp[i]);
        glVertex3f(glRampUp_indices[i][0], glRampUp_indices[i][1], 0);
        glVertex3f(glRampUp_indices[j][0], glRampUp_indices[j][1], 0);
        glVertex3f(glRampUp_indices[j][0], glRampUp_indices[j][1], ramp[j]);
        glEnd();
    }
}

float constexpr floor_height = .125;

GLvoid glFloor(int x, int y, int z) {
    static float constexpr glFloor_side[][10] = {
         .5  , -.207,
         .646, -.354,
         .5  , -.5  ,
         .354, -.646,
         .207, -.5  ,
        -.207, -.5  ,
        -.354, -.646,
        -.5  , -.5  ,
        -.646, -.354,
        -.5  , -.207,
        -.5  ,  .207,
        -.646,  .354,
        -.5  ,  .5  ,
        -.354,  .646,
        -.207,  .5  ,
         .207,  .5  ,
         .354,  .646,
         .5  ,  .5  ,
         .646,  .354,
         .5  ,  .207,
    };

    float constexpr _ = sqrt(.5);

    static float constexpr glFloor_normal[][3] = {
         1,  0,  0,
         _, -_,  0,
         0, -1,  0,
        -_, -_,  0,
        -1,  0,  0,
        -_,  _,  0,
         0,  1,  0,
         _,  _,  0,
    };

    TileDefinition structure[8];
    structure[0] = df.map.tileDefinition(x + 1, y    , z);
    structure[1] = df.map.tileDefinition(x + 1, y - 1, z);
    structure[2] = df.map.tileDefinition(x    , y - 1, z);
    structure[3] = df.map.tileDefinition(x - 1, y - 1, z);
    structure[4] = df.map.tileDefinition(x - 1, y    , z);
    structure[5] = df.map.tileDefinition(x - 1, y + 1, z);
    structure[6] = df.map.tileDefinition(x    , y + 1, z);
    structure[7] = df.map.tileDefinition(x + 1, y + 1, z);
    bool square_shaped = true; // optimize for square shaped floor tiles
    for (dword i = 0; i < 8; i += 2) {
        if (!structure[i].hasbase) {
            square_shaped = false;
            break;
        }
    }
    if (square_shaped) {
        glPushMatrix();
        glScalef(1, 1, floor_height);
        glCube();
        glPopMatrix();
    } else {
        glBegin(GL_TRIANGLE_FAN); // inner upper surface
        for (dword i = 0; i < 4; ++i) {
            glNormal3f(0, 0, 1);
            glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
            glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
        }
        glEnd();
        glBegin(GL_TRIANGLE_FAN); // inner lower surface
        for (int32 i = 3; i >= 0; --i) {
            glNormal3f(0, 0, -1);
            glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
            glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
        }
        glEnd();
        glBegin(GL_QUADS); // trivial sides
        for (dword i = 0; i < 4; ++i) {
            dword j = (i + 1) & 3;
            glNormal3fv(glFloor_normal[(i * 2 + 2) & 7]);
            glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
            glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
            glVertex3f(glFloor_side[j][0], glFloor_side[j][1], 0);
            glVertex3f(glFloor_side[j][0], glFloor_side[j][1], floor_height);
        }
        glEnd();
        for (dword i = 0; i < 4; ++i) {
            bool prevbase = structure[(i * 2    ) & 7].hasbase,
                 currbase = structure[(i * 2 + 1) & 7].hasbase,
                 nextbase = structure[(i * 2 + 2) & 7].hasbase,
                 prevblocked = structure[(i * 2    ) & 7].isblocked,
                 nextblocked = structure[(i * 2 + 2) & 7].isblocked,
                 downramp = structure[(i * 2    ) & 7].bodytype == BodyType::rampdown ||
                            structure[(i * 2 + 1) & 7].bodytype == BodyType::rampdown ||
                            structure[(i * 2 + 2) & 7].bodytype == BodyType::rampdown;
            if (prevbase || currbase || nextbase || downramp) {
                glBegin(GL_TRIANGLES);
                glNormal3f(0, 0, 1);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
                glNormal3f(0, 0, -1);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
                glEnd();
                if (!downramp && currbase && !prevbase && !nextblocked) { // extend
                    glBegin(GL_TRIANGLES);
                    glNormal3f(0, 0, 1);
                    glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
                    glVertex3f(glFloor_side[i][2], glFloor_side[i][3], floor_height);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                    glNormal3f(0, 0, -1);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                    glVertex3f(glFloor_side[i][2], glFloor_side[i][3], 0);
                    glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
                    glEnd();
                    glBegin(GL_QUADS);
                    glNormal3fv(glFloor_normal[(i * 2 - 1) & 7]);
                    glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
                    glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
                    glVertex3f(glFloor_side[i][2], glFloor_side[i][3], 0);
                    glVertex3f(glFloor_side[i][2], glFloor_side[i][3], floor_height);
                    glNormal3fv(glFloor_normal[(i * 2 + 1) & 7]);
                    glVertex3f(glFloor_side[i][2], glFloor_side[i][3], floor_height);
                    glVertex3f(glFloor_side[i][2], glFloor_side[i][3], 0);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                    glEnd();
                } else { // normal
                    glBegin(GL_QUADS);
                    glNormal3fv(glFloor_normal[(i * 2) & 7]);
                    glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
                    glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                    glEnd();
                }
                if (!downramp && currbase && !nextbase && !prevblocked) { // extend
                    glBegin(GL_TRIANGLES);
                    glNormal3f(0, 0, 1);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                    glVertex3f(glFloor_side[i][6], glFloor_side[i][7], floor_height);
                    glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
                    glNormal3f(0, 0, -1);
                    glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
                    glVertex3f(glFloor_side[i][6], glFloor_side[i][7], 0);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                    glEnd();
                    glBegin(GL_QUADS);
                    glNormal3fv(glFloor_normal[(i * 2 + 1) & 7]);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                    glVertex3f(glFloor_side[i][6], glFloor_side[i][7], 0);
                    glVertex3f(glFloor_side[i][6], glFloor_side[i][7], floor_height);
                    glNormal3fv(glFloor_normal[(i * 2 + 3) & 7]);
                    glVertex3f(glFloor_side[i][6], glFloor_side[i][7], floor_height);
                    glVertex3f(glFloor_side[i][6], glFloor_side[i][7], 0);
                    glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
                    glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
                    glEnd();
                } else { // normal
                    glBegin(GL_QUADS);
                    glNormal3fv(glFloor_normal[(i * 2 + 2) & 7]);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                    glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                    glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
                    glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
                    glEnd();
                }
            } else { // retract
                glBegin(GL_QUADS);
                glNormal3fv(glFloor_normal[(i * 2 + 1) & 7]);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
                glEnd();
            }
        }
    }
}

float constexpr wall_height = 1 - floor_height;

GLvoid glWall(int x, int y, int z) {
    static float constexpr glWall_corner[][2] = {
         .5, -.5,
        -.5, -.5,
        -.5,  .5,
         .5,  .5,
    };

    static float constexpr glWall_nocorner[][4] = {
         .5  , -.207,
         .207, -.5  ,
        -.207, -.5  ,
        -.5  , -.207,
        -.5  ,  .207,
        -.207,  .5  ,
         .207,  .5  ,
         .5  ,  .207,
    };

    static float constexpr glWall_normal_side[][3] = {
         1,  0,  0,
         0, -1,  0,
        -1,  0,  0,
         0,  1,  0,
    };

    float constexpr _ = sqrt(.5);

    static float constexpr glWall_normal_corner[][3] = {
         _, -_,  0,
        -_, -_,  0,
        -_,  _,  0,
         _,  _,  0,
    };

    TileDefinition me = df.map.tileDefinition(x, y, z);
    bool conn[4];
    if (me.hasconnection) {
        conn[0] = me.connection.east;
        conn[1] = me.connection.north;
        conn[2] = me.connection.west;
        conn[3] = me.connection.south;
    } else {
        TileDefinition s[4];
        s[0] = df.map.tileDefinition(x + 1, y    , z);
        s[1] = df.map.tileDefinition(x    , y - 1, z);
        s[2] = df.map.tileDefinition(x - 1, y    , z);
        s[3] = df.map.tileDefinition(x    , y + 1, z);
        for (int i = 0; i < 4; i++) {
            conn[i] = s[i].isblocked || (!s[i].treetype && s[i].isupslope);
        }
    }
    bool up_down = true;
    for (dword i = 0; i < 4; ++i) {
        if (!conn[i]) {
            up_down = false;
        }
    }
    if (up_down) { // optimize for walls fully surrounded by other walls & up slopes
        glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glVertex3f( .5,  .5, wall_height);
        glVertex3f( .5, -.5, wall_height);
        glVertex3f(-.5, -.5, wall_height);
        glVertex3f(-.5,  .5, wall_height);
        glNormal3f(0, 0, -1);
        glVertex3f(-.5,  .5, 0);
        glVertex3f(-.5, -.5, 0);
        glVertex3f( .5, -.5, 0);
        glVertex3f( .5,  .5, 0);
        glEnd();
        return;
    }
    bool corner[4];
    bool cube_shaped = true;
    for (dword i = 0; i < 4; ++i) {
        corner[i] = conn[i] | conn[(i + 1) & 3];
        cube_shaped &= corner[i];
    }
    if (cube_shaped) { // optimize for cube-shaped walls
        glPushMatrix();
        glScalef(1, 1, wall_height);
        glCube();
        glPopMatrix();
    } else {
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, 1);
        for (dword i = 0; i < 4; ++i) {
            if (corner[i]) {
                glVertex3f(glWall_corner[i][0], glWall_corner[i][1], wall_height);
            } else {
                glVertex3f(glWall_nocorner[i][0], glWall_nocorner[i][1], wall_height);
                glVertex3f(glWall_nocorner[i][2], glWall_nocorner[i][3], wall_height);
            }
        }
        glEnd();
        glBegin(GL_TRIANGLE_FAN);
        glNormal3f(0, 0, -1);
        for (int32 i = 3; i >= 0; i--) {
            if (corner[i]) {
                glVertex3f(glWall_corner[i][0], glWall_corner[i][1], 0);
            } else {
                glVertex3f(glWall_nocorner[i][2], glWall_nocorner[i][3], 0);
                glVertex3f(glWall_nocorner[i][0], glWall_nocorner[i][1], 0);
            }
        }
        glEnd();
        glBegin(GL_QUADS);
        for (dword i = 0; i < 4; ++i) {
            dword j = (i + 1) & 3;
            glNormal3fv(glWall_normal_side[j]);
            if (corner[j]) {
                glVertex3f(glWall_corner[j][0], glWall_corner[j][1], 0);
                glVertex3f(glWall_corner[j][0], glWall_corner[j][1], wall_height);
            } else {
                glVertex3f(glWall_nocorner[j][0], glWall_nocorner[j][1], 0);
                glVertex3f(glWall_nocorner[j][0], glWall_nocorner[j][1], wall_height);
            }
            if (corner[i]) {
                glVertex3f(glWall_corner[i][0], glWall_corner[i][1], wall_height);
                glVertex3f(glWall_corner[i][0], glWall_corner[i][1], 0);
            } else {
                glVertex3f(glWall_nocorner[i][2], glWall_nocorner[i][3], wall_height);
                glVertex3f(glWall_nocorner[i][2], glWall_nocorner[i][3], 0);

                glNormal3fv(glWall_normal_corner[i]);
                glVertex3f(glWall_nocorner[i][2], glWall_nocorner[i][3], 0);
                glVertex3f(glWall_nocorner[i][2], glWall_nocorner[i][3], wall_height);
                glVertex3f(glWall_nocorner[i][0], glWall_nocorner[i][1], wall_height);
                glVertex3f(glWall_nocorner[i][0], glWall_nocorner[i][1], 0);
            }
        }
        glEnd();
    }
}

GLvoid glStairUp() {
    static Model model("models/tiles/stair_up.mdl");
    model.glRender();
}

GLvoid glStairDown(int x, int y, int z) {
    static float constexpr glFloor_side[][10] = {
         .5  , -.207,
         .646, -.354,
         .5  , -.5  ,
         .354, -.646,
         .207, -.5  ,
        -.207, -.5  ,
        -.354, -.646,
        -.5  , -.5  ,
        -.646, -.354,
        -.5  , -.207,
        -.5  ,  .207,
        -.646,  .354,
        -.5  ,  .5  ,
        -.354,  .646,
        -.207,  .5  ,
         .207,  .5  ,
         .354,  .646,
         .5  ,  .5  ,
         .646,  .354,
         .5  ,  .207,
    };

    float constexpr _ = sqrt(.5);

    static float constexpr glFloor_normal[][3] = {
         1,  0,  0,
         _, -_,  0,
         0, -1,  0,
        -_, -_,  0,
        -1,  0,  0,
        -_,  _,  0,
         0,  1,  0,
         _,  _,  0,
    };

    static float constexpr glStairDown_inner[][2] = {
          0, -.3 ,
        -.3, -.3 ,
        -.3,  .3,
          0,  .3,
    };

    bool base[8];
    base[0] = df.map.tileDefinition(x + 1, y    , z).hasbase;
    base[1] = df.map.tileDefinition(x + 1, y - 1, z).hasbase;
    base[2] = df.map.tileDefinition(x    , y - 1, z).hasbase;
    base[3] = df.map.tileDefinition(x - 1, y - 1, z).hasbase;
    base[4] = df.map.tileDefinition(x - 1, y    , z).hasbase;
    base[5] = df.map.tileDefinition(x - 1, y + 1, z).hasbase;
    base[6] = df.map.tileDefinition(x    , y + 1, z).hasbase;
    base[7] = df.map.tileDefinition(x + 1, y + 1, z).hasbase;
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) & 3;
        glBegin(GL_TRIANGLES);
        glNormal3f(0, 0, 1);
        glVertex3f(glStairDown_inner[i][0], glStairDown_inner[i][1], floor_height);
        glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
        glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
        glNormal3f(0, 0, -1);
        glVertex3f(glStairDown_inner[i][0], glStairDown_inner[i][1], 0);
        glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
        glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
        glEnd();
        glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glVertex3f(glStairDown_inner[i][0], glStairDown_inner[i][1], floor_height);
        glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
        glVertex3f(glFloor_side[j][0], glFloor_side[j][1], floor_height);
        glVertex3f(glStairDown_inner[j][0], glStairDown_inner[j][1], floor_height);
        glNormal3f(0, 0, -1);
        glVertex3f(glStairDown_inner[j][0], glStairDown_inner[j][1], 0);
        glVertex3f(glFloor_side[j][0], glFloor_side[j][1], 0);
        glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
        glVertex3f(glStairDown_inner[i][0], glStairDown_inner[i][1], 0);
        glEnd();
    }
    glBegin(GL_QUADS);
    for (int i = 0; i < 4; i++) {
        int j = (i + 1) & 3;
        glNormal3fv(glFloor_normal[(i * 2 + 2) & 7]);
        glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
        glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
        glVertex3f(glFloor_side[j][0], glFloor_side[j][1], 0);
        glVertex3f(glFloor_side[j][0], glFloor_side[j][1], floor_height);
        glNormal3fv(glFloor_normal[(i * 2 + 6) & 7]);
        glVertex3f(glStairDown_inner[i][0], glStairDown_inner[i][1], 0);
        glVertex3f(glStairDown_inner[i][0], glStairDown_inner[i][1], floor_height);
        glVertex3f(glStairDown_inner[j][0], glStairDown_inner[j][1], floor_height);
        glVertex3f(glStairDown_inner[j][0], glStairDown_inner[j][1], 0);
    }
    glEnd();
    for (int i = 0; i < 4; i++) {
        bool prev = base[(i * 2    ) & 7],
             curr = base[(i * 2 + 1) & 7],
             next = base[(i * 2 + 2) & 7];
        if (prev || curr || next) {
            glBegin(GL_TRIANGLES);
            glNormal3f(0, 0, 1);
            glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
            glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
            glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
            glNormal3f(0, 0, -1);
            glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
            glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
            glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
            glEnd();
            if (curr && !prev) {
                glBegin(GL_TRIANGLES);
                glNormal3f(0, 0, 1);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
                glVertex3f(glFloor_side[i][2], glFloor_side[i][3], floor_height);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                glNormal3f(0, 0, -1);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                glVertex3f(glFloor_side[i][2], glFloor_side[i][3], 0);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
                glEnd();
                glBegin(GL_QUADS);
                glNormal3fv(glFloor_normal[(i * 2 - 1) & 7]);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
                glVertex3f(glFloor_side[i][2], glFloor_side[i][3], 0);
                glVertex3f(glFloor_side[i][2], glFloor_side[i][3], floor_height);
                glNormal3fv(glFloor_normal[(i * 2 + 1) & 7]);
                glVertex3f(glFloor_side[i][2], glFloor_side[i][3], floor_height);
                glVertex3f(glFloor_side[i][2], glFloor_side[i][3], 0);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                glEnd();
            } else {
                glBegin(GL_QUADS);
                glNormal3fv(glFloor_normal[(i * 2) & 7]);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
                glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                glEnd();
            }
            if (curr && !next) {
                glBegin(GL_TRIANGLES);
                glNormal3f(0, 0, 1);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                glVertex3f(glFloor_side[i][6], glFloor_side[i][7], floor_height);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
                glNormal3f(0, 0, -1);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
                glVertex3f(glFloor_side[i][6], glFloor_side[i][7], 0);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                glEnd();
                glBegin(GL_QUADS);
                glNormal3fv(glFloor_normal[(i * 2 + 1) & 7]);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                glVertex3f(glFloor_side[i][6], glFloor_side[i][7], 0);
                glVertex3f(glFloor_side[i][6], glFloor_side[i][7], floor_height);
                glNormal3fv(glFloor_normal[(i * 2 + 3) & 7]);
                glVertex3f(glFloor_side[i][6], glFloor_side[i][7], floor_height);
                glVertex3f(glFloor_side[i][6], glFloor_side[i][7], 0);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
                glEnd();
            } else {
                glBegin(GL_QUADS);
                glNormal3fv(glFloor_normal[(i * 2 + 2) & 7]);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], floor_height);
                glVertex3f(glFloor_side[i][4], glFloor_side[i][5], 0);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
                glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
                glEnd();
            }
        } else {
            glBegin(GL_QUADS);
            glNormal3fv(glFloor_normal[(i * 2 + 1) & 7]);
            glVertex3f(glFloor_side[i][0], glFloor_side[i][1], floor_height);
            glVertex3f(glFloor_side[i][0], glFloor_side[i][1], 0);
            glVertex3f(glFloor_side[i][8], glFloor_side[i][9], 0);
            glVertex3f(glFloor_side[i][8], glFloor_side[i][9], floor_height);
            glEnd();
        }
    }
}

GLvoid glStairBoth(int x, int y, int z) {
    glStairDown(x, y, z);
    glPushMatrix();
    glTranslatef(0, 0, floor_height);
    glStairUp();
    glPopMatrix();
}

bool glApplySpecular(int16 material, int32 species) {
    static float constexpr spec_metal[3] = {.3, .3, .3}, spec_glass[3] = {.1, .1, .1};
    float constexpr ang_metal = M_PI / 8, ang_glass = M_PI / 4;
    if (df.isMetal(material, species)) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec_metal);
        glMaterialf(GL_FRONT, GL_SHININESS, ang_metal);
        return true;
    } else if (df.isGlass(material, species)) {
        glMaterialfv(GL_FRONT, GL_SPECULAR, spec_glass);
        glMaterialf(GL_FRONT, GL_SHININESS, ang_glass);
        return true;
    } else {
        return false;
    }
}

GLvoid glResetSpecular() {
    static float constexpr spec_0[3] = {0, 0, 0};
    float constexpr ang_0 = 0;
    glMaterialfv(GL_FRONT, GL_SPECULAR, spec_0);
    glMaterialf(GL_FRONT, GL_SHININESS, ang_0);
}

GLvoid glTree(Vegetation & veg, renderrange::ziterator const & range) {
    int tree_x_start = veg.location.x - (veg.dimension->x / 2),
        tree_x_end   = tree_x_start + veg.dimension->x,
        tree_y_start = veg.location.y - (veg.dimension->y / 2),
        tree_y_end   = tree_y_start + veg.dimension->y,
        tree_z_start = veg.location.z - veg.dimension->root_depth,
        tree_z_end   = veg.location.z + veg.dimension->z;
    VegetationRaw & raw = *df.map.vegetation_raw[veg.species];
    byte ** root_occupancy = veg.dimension->root_occupancy,
         ** body_occupancy = veg.dimension->body_occupancy;
    for (int z = tree_z_end - 1; z >= tree_z_start; --z) {
        for (int x = tree_x_start; x < tree_x_end; ++x) {
            for (int y = tree_y_start; y < tree_y_end; ++y) {
                int local_x = x - tree_x_start,
                    local_y = y - tree_y_start,
                    local_z = z - veg.location.z;
                byte occupancy;
                if (x == veg.location.x && y == veg.location.y && z == veg.location.z) {
                    occupancy = 1; // the origin tile seems always included
                } else if (local_z < 0 && root_occupancy && root_occupancy[-local_z-1]){
                    occupancy = root_occupancy[-local_z-1][local_x + local_y * veg.dimension->y];
                } else if (local_z >= 0 && body_occupancy && body_occupancy[local_z]) {
                    occupancy = body_occupancy[local_z][local_x + local_y * veg.dimension->y];
                } else {
                    occupancy = 0;
                }
                int16 shownMaterial = -1;
                int32 shownSpecies = -1;
                if ((occupancy & 0x7F) && // occupancy != 0 (no vegetation) nor 80 (other vegetation)
                    !df.map.tileInfo(x, y, z).invisible &&
                    range.contains(x, y, z)) {
                    TileStructure::Enum type = df.map.tileStructure(x, y, z);
                    TileDefinition def = tileDefinition(type);
                    glPushMatrix();
                    glTranslatef(x, y, z);
                    if (def.isalive) {
                        if (&raw) {
                            bool has_growth = false;
                            dword last_priority = 0;
                            int32 ticks = Vegetation::getGrowthTicks(x, y);
                            for (auto curr = raw.growth.rbegin(), end = raw.growth.rend(); curr < end; ++curr) {
                                VegetationRaw::GrowthRaw & growth = **curr;
                                  // it's actually like this in the binary. Hmm...
                                  if (&growth &&
                                         (growth.time_start < 0 ||
                                         (growth.time_start > growth.time_end && ticks >= growth.time_end) ||
                                         (growth.time_start <= ticks && ticks <= growth.time_end))) {
                                    bool grows;
                                    switch (def.treetype) {
                                    using namespace TreeType;
                                    case twigs:
                                        grows = growth.host_tile.twigs;
                                        break;
                                    case branches:
                                        grows = growth.host_tile.branches;
                                        break;
                                    case thick_branches:
                                        grows = growth.host_tile.heavy_branches;
                                        break;
                                    case trunk:
                                        grows = growth.host_tile.trunk;
                                        break;
                                    case root:
                                        grows = growth.host_tile.roots;
                                        break;
                                    case cap:
                                        grows = growth.host_tile.cap;
                                        break;
                                    default:
                                        grows = false;
                                    }
                                    if (grows) {
                                        for (auto curr = growth.print.rbegin(), end = growth.print.rend(); curr < end; ++curr) {
                                            VegetationRaw::GrowthPrint & print = **curr;
                                            if (&print && (!has_growth || print.priority > last_priority) &&
                                                              (print.time_start < 0 ||
                                                              (print.time_start > print.time_end && ticks >= print.time_end) ||
                                                              (print.time_start <= ticks && ticks <= print.time_end))) {
                                                has_growth = true;
                                                last_priority = print.priority;
                                                // growth color
                                                shownMaterial = growth.material;
                                                shownSpecies = growth.species;
                                                glColor3fv(df.getColorOfMaterial(growth.material, growth.species, MaterialState::solid, print.color_fore, print.color_fore_bright, true));
                                            }
                                        }
                                    }
                                }
                            }
                            if (!has_growth) { // structural color
                                shownMaterial = raw.basic_material;
                                shownSpecies = raw.basic_species;
                                glColor3fv(df.getColorOfMaterial(raw.basic_material, raw.basic_species, MaterialState::solid, TemplateColorType::basic, false));
                                //todo tree_color?
                            }
                        } else { // default color
                            glColor3f(.59, .29,  0);
                        }
                    } else { // death color. VegetationRaw::dead_tree_color doesn't seem to be used by the game, so we don't use it either
                        if (&raw) {
                            shownMaterial = raw.basic_material;
                            shownSpecies = raw.basic_species;
                        }
                        glColor3f(.39, .27,  0);
                    }
                    bool reset = glApplySpecular(shownMaterial, shownSpecies);
                    // todo cap_pointed
                    if (def.isupslope) {
                        glRampUp(x, y, z);
                    } else if (def.connection.north == 0 && def.connection.south == 0 && def.connection.west == 0 && def.connection.east == 0) {
                        glCube();
                    } else {
                        dword _3_cnt = (def.connection.north == 3 ? 1 : 0) +
                                       (def.connection.south == 3 ? 1 : 0) +
                                       (def.connection.west  == 3 ? 1 : 0) +
                                       (def.connection.east  == 3 ? 1 : 0);
                        if (_3_cnt == 2) {
                            float x = -.25, y = -.25;
                            if (def.connection.south == 3) {
                                x = .25;
                            }
                            if (def.connection.east == 3) {
                                y = .25;
                            }
                            glPushMatrix();
                            glTranslatef(x, y, 0);
                            glScalef(.5, .5, 1);
                            glCube();
                            glPopMatrix();
                        } else if (_3_cnt == 1) {
                            float x = 0, y = 0;
                            float sx = 1, sy = 1;
                            if (def.connection.north == 3) {
                                x = -.25;
                                sy = .5;
                            } else if (def.connection.south == 3) {
                                x = .25;
                                sy = .5;
                            }
                            if (def.connection.west == 3) {
                                y = -.25;
                                sx = .5;
                            } else if (def.connection.east == 3) {
                                y = .25;
                                sx = .5;
                            }
                            glPushMatrix();
                            glTranslatef(x, y, 0);
                            glScalef(sx, sy, 1);
                            glCube();
                            glPopMatrix();
                        }
                        if (_3_cnt < 4) {
                            dword constexpr vertex_cnt = 8;
                            static float constexpr shape[vertex_cnt][2] = {
                                cos(M_PI * 2 / vertex_cnt * 0), sin(M_PI * 2 / vertex_cnt * 0),
                                cos(M_PI * 2 / vertex_cnt * 1), sin(M_PI * 2 / vertex_cnt * 1),
                                cos(M_PI * 2 / vertex_cnt * 2), sin(M_PI * 2 / vertex_cnt * 2),
                                cos(M_PI * 2 / vertex_cnt * 3), sin(M_PI * 2 / vertex_cnt * 3),
                                cos(M_PI * 2 / vertex_cnt * 4), sin(M_PI * 2 / vertex_cnt * 4),
                                cos(M_PI * 2 / vertex_cnt * 5), sin(M_PI * 2 / vertex_cnt * 5),
                                cos(M_PI * 2 / vertex_cnt * 6), sin(M_PI * 2 / vertex_cnt * 6),
                                cos(M_PI * 2 / vertex_cnt * 7), sin(M_PI * 2 / vertex_cnt * 7),
                            };

                            float north = def.connection.north == 1 ? .125 : def.connection.north == 2 ? .25 : 0;
                            float south = def.connection.south == 1 ? .125 : def.connection.south == 2 ? .25 : 0;
                            float west = def.connection.west == 1 ? .125 : def.connection.west == 2 ? .25 : 0;
                            float east = def.connection.east == 1 ? .125 : def.connection.east == 2 ? .25 : 0;
                            dword cnt = (north != 0 ? 1 : 0) + (south != 0 ? 1 : 0) + (west != 0 ? 1 : 0) + (east != 0 ? 1 : 0);
                            float average = (north + south + west + east) / cnt;
                            if (!((north != 0 && south != 0) || (west != 0 && east != 0))) {
                                glPushMatrix();
                                float scale = average * 2;
                                glTranslatef(0, 0, .5 - average);
                                glScalef(scale, scale, scale);
                                glSphere();
                                glPopMatrix();
                            }
                            glPushMatrix();
                            glTranslatef(0, 0, .5);
                            if (north != 0) {
                                float nr = .5, ny = north - average, sr = sqrt(nr * nr + ny * ny);
                                nr /= sr; ny /= sr;
                                if (south == 0) {
                                    glBegin(GL_TRIANGLE_FAN);
                                    glNormal3f(0, 1, 0);
                                    for (dword i = 0; i < vertex_cnt; ++i) {
                                        glVertex3f(shape[i][0] * average, 0, shape[i][1] * average);
                                    }
                                    glEnd();
                                }
                                glBegin(GL_TRIANGLE_FAN);
                                glNormal3f(0, -1, 0);
                                for (int32 i = vertex_cnt - 1; i >= 0; --i) {
                                    glVertex3f(shape[i][0] * north, -.5, shape[i][1] * north);
                                }
                                glEnd();
                                glBegin(GL_TRIANGLE_STRIP);
                                for (dword i = 0; i < vertex_cnt; ++i) {
                                    glNormal3f(shape[i][0] * nr, ny, shape[i][1] * nr);
                                    glVertex3f(shape[i][0] * average, 0, shape[i][1] * average);
                                    glVertex3f(shape[i][0] * north, -.5, shape[i][1] * north);
                                }
                                glNormal3f(shape[0][0] * nr, ny, shape[0][1] * nr);
                                glVertex3f(shape[0][0] * average, 0, shape[0][1] * average);
                                glVertex3f(shape[0][0] * north, -.5, shape[0][1] * north);
                                glEnd();
                            }
                            if (south != 0) {
                                float nr = .5, ny = average - south, sr = sqrt(nr * nr + ny * ny);
                                nr /= sr; ny /= sr;
                                if (north == 0) {
                                    glBegin(GL_TRIANGLE_FAN);
                                    glNormal3f(0, -1, 0);
                                    for (int32 i = vertex_cnt - 1; i >= 0; --i) {
                                        glVertex3f(shape[i][0] * average, 0, shape[i][1] * average);
                                    }
                                    glEnd();
                                }
                                glBegin(GL_TRIANGLE_FAN);
                                glNormal3f(0, 1, 0);
                                for (dword i = 0; i < vertex_cnt; ++i) {
                                    glVertex3f(shape[i][0] * south, .5, shape[i][1] * south);
                                }
                                glEnd();
                                glBegin(GL_TRIANGLE_STRIP);
                                for (dword i = 0; i < vertex_cnt; ++i) {
                                    glNormal3f(shape[i][0] * nr, ny, shape[i][1] * nr);
                                    glVertex3f(shape[i][0] * south, .5, shape[i][1] * south);
                                    glVertex3f(shape[i][0] * average, 0, shape[i][1] * average);
                                }
                                glNormal3f(shape[0][0] * nr, ny, shape[0][1] * nr);
                                glVertex3f(shape[0][0] * south, .5, shape[0][1] * south);
                                glVertex3f(shape[0][0] * average, 0, shape[0][1] * average);
                                glEnd();
                            }
                            if (west != 0) {
                                float nr = .5, nx = west - average, sr = sqrt(nr * nr  + nx * nx);
                                nr /= sr, nx /= sr;
                                if (east == 0) {
                                    glBegin(GL_TRIANGLE_FAN);
                                    glNormal3f(1, 0, 0);
                                    for (int32 i = vertex_cnt - 1; i >= 0; --i) {
                                        glVertex3f(0, shape[i][0] * average, shape[i][1] * average);
                                    }
                                    glEnd();
                                }
                                glBegin(GL_TRIANGLE_FAN);
                                glNormal3f(-1, 0, 0);
                                for (dword i = 0; i < vertex_cnt; ++i) {
                                    glVertex3f(-.5, shape[i][0] * west, shape[i][1] * west);
                                }
                                glEnd();
                                glBegin(GL_TRIANGLE_STRIP);
                                for (dword i = 0; i < vertex_cnt; ++i) {
                                    glNormal3f(nx, shape[i][0] * nr, shape[i][1] * nr);
                                    glVertex3f(-.5, shape[i][0] * west, shape[i][1] * west);
                                    glVertex3f(0, shape[i][0] * average, shape[i][1] * average);
                                }
                                glNormal3f(nx, shape[0][0] * nr, shape[0][1] * nr);
                                glVertex3f(-.5, shape[0][0] * west, shape[0][1] * west);
                                glVertex3f(0, shape[0][0] * average, shape[0][1] * average);
                                glEnd();
                            }
                            if (east != 0) {
                                float nr = .5, nx = average - east, sr = sqrt(nr * nr  + nx * nx);
                                nr /= sr, nx /= sr;
                                if (west == 0) {
                                    glBegin(GL_TRIANGLE_FAN);
                                    glNormal3f(-1, 0, 0);
                                    for (dword i = 0; i < vertex_cnt; ++i) {
                                        glVertex3f(0, shape[i][0] * average, shape[i][1] * average);
                                    }
                                    glEnd();
                                }
                                glBegin(GL_TRIANGLE_FAN);
                                glNormal3f(1, 0, 0);
                                for (int32 i = vertex_cnt; i >= 0; --i) {
                                    glVertex3f(.5, shape[i][0] * east, shape[i][1] * east);
                                }
                                glEnd();
                                glBegin(GL_TRIANGLE_STRIP);
                                for (dword i = 0; i < vertex_cnt; ++i) {
                                    glNormal3f(nx, shape[i][0] * nr, shape[i][1] * nr);
                                    glVertex3f(0, shape[i][0] * average, shape[i][1] * average);
                                    glVertex3f(.5, shape[i][0] * east, shape[i][1] * east);
                                }
                                glNormal3f(nx, shape[0][0] * nr, shape[0][1] * nr);
                                glVertex3f(0, shape[0][0] * average, shape[0][1] * average);
                                glVertex3f(.5, shape[0][0] * east, shape[0][1] * east);
                                glEnd();
                            }
                            glPopMatrix();
                        }
                    }
                    if (reset) {
                        glResetSpecular();
                    }
                    glPopMatrix();
                }
            }
        }
    }
}

GLvoid glSapling(Vegetation & veg, bool alive) {
    VegetationRaw & raw = *df.map.vegetation_raw[veg.species];
    glPushMatrix();
    glScalef(.1, .1, .5);
    int16 shownMaterial = raw.basic_material;
    int32 shownSpecies = raw.basic_species;
    if (alive) {
        if (&raw) { // raw color
            glColor3fv(df.getColorOfMaterial(raw.basic_material, raw.basic_species, MaterialState::solid, raw.sapling_color_fore, raw.sapling_color_back, true));
        } else { // default color
            glColor3f(.59, .29,  0);
        }
    } else { // death color
        if (&raw) { // raw color
            glColor3fv(df.getColorOfMaterial(raw.basic_material, raw.basic_species, MaterialState::solid, raw.dead_sapling_color_fore, raw.dead_sapling_color_back, true));
        } else { // default color
            glColor3f(.39, .27,  0);
        }
    }
    bool reset = glApplySpecular(shownMaterial, shownSpecies);
    glCylinder();
    if (reset) {
        glResetSpecular();
    }
    shownMaterial = -1;
    shownSpecies = -1;
    bool proceed = true;
    bool has_growth = false;
    if (&raw) {
        dword last_priority = 0;
        int32 ticks = veg.getGrowthTicks();
        for (auto curr = raw.growth.rbegin(), end = raw.growth.rend(); proceed && curr < end; ++curr) {
            VegetationRaw::GrowthRaw & growth = **curr;
            if (&growth &&
                     (growth.time_start < 0 ||
                     (growth.time_start > growth.time_end && ticks >= growth.time_end) ||
                     (growth.time_start <= ticks && ticks <= growth.time_end))) {
                if (growth.host_tile.sapling) {
                    for (auto curr = growth.print.rbegin(), end = growth.print.rend();
                         proceed && curr < end; ++curr) {
                        VegetationRaw::GrowthPrint & print = **curr;
                        if (&print && (!has_growth || print.priority > last_priority) &&
                                (print.time_start < 0 ||
                                (print.time_start > print.time_end && ticks >= print.time_end) ||
                                (print.time_start <= ticks && ticks <= print.time_end))) {
                            has_growth = true;
                            if (alive) {
                                last_priority = print.priority;
                                // growth color
                                shownMaterial = growth.material;
                                shownSpecies = growth.species;
                                glColor3fv(df.getColorOfMaterial(growth.material, growth.species, MaterialState::solid, print.color_fore, print.color_fore_bright, true));
                            } else { // death color
                                shownMaterial = raw.basic_material;
                                shownSpecies = raw.basic_species;
                                glColor3fv(df.getColorOfMaterial(raw.basic_material, raw.basic_species, MaterialState::solid, raw.dead_sapling_color_fore, raw.dead_sapling_color_back, true));
                                proceed = false;
                            }
                        }
                    }
                }
            }
        }
    }
    if (has_growth) {
        glScalef(5, 5, 1);
        glTranslatef(0, 0, 1 - floor_height);
        reset = glApplySpecular(shownMaterial, shownSpecies);
        glSphere();
        if (reset) {
            glResetSpecular();
        }
    }
    glPopMatrix();
}

GLvoid glShrub(Vegetation & veg, bool alive) {
    VegetationRaw & raw = *df.map.vegetation_raw[veg.species];
    int16 shownMaterial = -1;
    int32 shownSpecies = -1;
    if (alive) {
        if (&raw) {
            bool has_growth = false;
            dword last_priority = 0;
            int32 ticks = veg.getGrowthTicks();
            for (auto curr = raw.growth.rbegin(), end = raw.growth.rend(); curr < end; ++curr) {
                VegetationRaw::GrowthRaw & growth = **curr;
                if (&growth &&
                         (growth.time_start < 0 ||
                         (growth.time_start > growth.time_end && ticks >= growth.time_end) ||
                         (growth.time_start <= ticks && ticks <= growth.time_end))) {
                    for (auto curr = growth.print.rbegin(), end = growth.print.rend(); curr < end; ++curr) {
                        VegetationRaw::GrowthPrint & print = **curr;
                        if (&print && (!has_growth || print.priority > last_priority) &&
                                (print.time_start < 0 ||
                                (print.time_start > print.time_end && ticks >= print.time_end) ||
                                (print.time_start <= ticks && ticks <= print.time_end))) {
                            has_growth = true;
                            last_priority = print.priority;
                            // growth color
                            shownMaterial = growth.material;
                            shownSpecies = growth.species;
                            glColor3fv(df.getColorOfMaterial(growth.material, growth.species, MaterialState::solid, print.color_fore, print.color_fore_bright, true));
                        }
                    }
                }
            }
            if (!has_growth) { // structural color
                shownMaterial = raw.basic_material;
                shownSpecies = raw.basic_species;
                glColor3fv(df.getColorOfMaterial(raw.basic_material, raw.basic_species, MaterialState::solid, raw.shrub_color_fore, raw.shrub_color_fore_bright, true));
            }
        } else { // default color
            glColor3f(.10, .40, .15);
        }
    } else { // death color
        if (&raw) {
            shownMaterial = raw.basic_material;
            shownSpecies = raw.basic_species;
            glColor3fv(df.getColorOfMaterial(raw.basic_material, raw.basic_species, MaterialState::solid, raw.dead_sapling_color_fore, raw.dead_sapling_color_back, true));
        } else {
            glColor3f(.39, .27,  0);
        }
    }
    bool reset = glApplySpecular(shownMaterial, shownSpecies);
    glBegin(GL_TRIANGLES);
    glNormal3f(  1,   0,  0);
    glVertex3f(  0, -.5,  0);
    glVertex3f(  0,   0, .5);
    glVertex3f(  0,  .5,  0);
    glNormal3f( -1,   0,  0);
    glVertex3f(  0,  .5,  0);
    glVertex3f(  0,   0, .5);
    glVertex3f(  0, -.5,  0);
    glNormal3f(  0,   1,  0);
    glVertex3f( .5,   0,  0);
    glVertex3f(  0,   0, .5);
    glVertex3f(-.5,   0,  0);
    glNormal3f(  0,  -1,  0);
    glVertex3f(-.5,   0,  0);
    glVertex3f(  0,   0, .5);
    glVertex3f( .5,   0,  0);
    glEnd();
    if (reset) {
        glResetSpecular();
    }
}

GLvoid glCreature(Creature const & creature) {
    if (!creature.flags.dead_or_missing) {
        glColor3f(0, 1, 1);
        glCube();
    }
}

GLvoid glPebble(dword tile_x, dword tile_y, dword tile_z) {
    dword count = 4;
    float constexpr size = .1;
    srand(0xAAAAAAAA ^ tile_x);
    srand(rand() ^ tile_y);
    srand(rand() ^ tile_z);
    while (count--) {
        float _0 = -.5 + size / 2,
              _1 =  .5 - size / 2;
        float x = rand() / (float) RAND_MAX * (_1 - _0) + _0;
        float y = rand() / (float) RAND_MAX * (_1 - _0) + _0;
        glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(size, size, size);
        glCube();
        glPopMatrix();
    }
}

GLvoid glBoulder(dword tile_x, dword tile_y, dword tile_z) {
    dword count = 4;
    float constexpr size = .3;
    srand(0xAAAAAAAA ^ tile_x);
    srand(rand() ^ tile_y);
    srand(rand() ^ tile_z);
    while (count--) {
        float _0 = -.5 + size / 2,
              _1 =  .5 - size / 2;
        float x = rand() / (float) RAND_MAX * (_1 - _0) + _0;
        float y = rand() / (float) RAND_MAX * (_1 - _0) + _0;
        float height = rand() / (float) RAND_MAX * (wall_height / 2) + (wall_height / 2);
        glPushMatrix();
        glTranslatef(x, y, 0);
        glScalef(size, size, height);
        glCube();
        glPopMatrix();
    }
}

GLvoid glCluster(dword tile_x, dword tile_y, dword tile_z, dword count, float size) {
    srand(0xAAAAAAAA ^ tile_x);
    srand(rand() ^ tile_y);
    srand(rand() ^ tile_z);
    while (count--) {
        dword face = ((dword) rand()) % 6;
        float _z0 = -.5 + size / 2 - min(size / 2, .05f),
              _z1 = -_z0;
        float _x = rand() / (float) RAND_MAX * (_z1 - _z0) + _z0;
        float _y = rand() / (float) RAND_MAX * (_z1 - _z0) + _z0;
        float x, y, z;
        switch (face) {
        case 0: // west, -x
            x = _z0;
            y = _x;
            z = _y;
            break;
        case 1: // east, +x
            x = _z1;
            y = _x;
            z = _y;
            break;
        case 2: // north, -y
            x = _x;
            y = _z0;
            z = _y;
            break;
        case 3: // south, +y
            x = _x;
            y = _z1;
            z = _y;
            break;
        case 4: // down, -z
            x = _x;
            y = _y;
            z = _z0;
            break;
        case 5: // up, +z
            x = _x;
            y = _y;
            z = _z1;
            break;
        }
        z += .5 - size / 2;
        glPushMatrix();
        glTranslatef(x, y, z);
        glScalef(size, size, size);
        glCube();
        glPopMatrix();
    }
}

GLvoid glPaintBlock2d(Block2d & block, renderrange::ziterator const & range) {
    for (auto curr = block.vegetations.begin(), end = block.vegetations.end(); curr < end; ++curr) {
        Vegetation & veg = **curr;
        if (veg.dimension == nullptr) { // not tree
            if (!df.map.tileInfo(veg.location.x, veg.location.y, veg.location.z).invisible &&
                    range.contains(veg.location.x, veg.location.y, veg.location.z)) {
                TileStructure::Enum structure = df.map.tileStructure(veg.location.x, veg.location.y, veg.location.z);
                switch (structure) {
                case TileStructure::sapling:
                case TileStructure::dead_sapling:
                    glPushMatrix();
                    glTranslatef(veg.location.x, veg.location.y, veg.location.z);
                    glSapling(veg, structure == TileStructure::sapling);
                    glPopMatrix();
                    break;
                case TileStructure::shrub:
                case TileStructure::dead_shrub:
                    glPushMatrix();
                    glTranslatef(veg.location.x, veg.location.y, veg.location.z);
                    glShrub(veg, structure == TileStructure::shrub);
                    glPopMatrix();
                    break;
                default:
                    break;
                }
            }
        } else {
            glTree(veg, range);
        }
    }
}

GLvoid glPaintBlock3d(Block3d & block) {
    for (auto curr = block.items.begin(), end = block.items.end(); curr < end; ++curr) {
        dword index = *curr;
        Item & item = *df.map.items.find([index] (Item * const & item) -> int {return index - item->index;});
        if (&item) {
            int x = item.location.x, y = item.location.y, z = item.location.z;
            if (
//                x >= block.location.x && x <= block.location.x + 15 &&
//                y >= block.location.y && y <= block.location.y + 15 &&
                z == block.location.z &&
                !df.map.tileInfo(x, y, z).invisible) {
                glPushMatrix();
                if (df.map.tileDefinition(x, y, z).isupslope) {
                    glTranslatef(x, y, z + floor_height + .5);
                } else {
                    glTranslatef(x, y, z + floor_height);
                }

                int16 material = item.vtable->getMaterial(&item);
                int32 species = item.vtable->getSpecies(&item);

                glColor3fv(df.getColorOfMaterial(material, species, MaterialState::solid, TemplateColorType::build, false));
                bool reset = glApplySpecular(material, species);

                switch (item.vtable->getItemType(&item)) {
                using namespace ItemType;
                case bar: {
                    static Model model("models/bar.mdl");
                    model.glRender();
                }   break;
                case gem_small: {
                    GemCut::Enum type = item.vtable->getGemCut(&item);
                    static Model model[] = {
                        "models/gem_small/gizzard_stones.mdl",
                        "models/gem_small/smooth_pebbles.mdl",
                        "models/gem_small/oval_cabochon.mdl",
                        "models/gem_small/round_cabochon.mdl",
                        "models/gem_small/cushion_cabochon.mdl",
                        "models/gem_small/rectangular_cabochon.mdl",
                        "models/gem_small/point_cut.mdl",
                        "models/gem_small/table_cut.mdl",
                        "models/gem_small/single_cut.mdl",
                        "models/gem_small/rose_cut.mdl",
                        "models/gem_small/briolette_cut.mdl",
                        "models/gem_small/emerald_cut.mdl",
                        "models/gem_small/marquise_cut.mdl",
                        "models/gem_small/oval_cut.mdl",
                        "models/gem_small/pear_cut.mdl",
                        "models/gem_small/square_brilliant_cut.mdl",
                        "models/gem_small/radiant_cut.mdl",
                        "models/gem_small/trillion_cut.mdl",
                        "models/gem_small/round_brilliant_cut.mdl",
                        "models/gem_small/baguette_cut.mdl",
                        "models/gem_small/tapered_baguette_cut.mdl",
                        "models/gem_small/cushion_cut.mdl",
                        "models/gem_small/octagon_cut.mdl",
                        "models/gem_small/square_cut.mdl",
                    };
                    if (type >= GemCut::gizzard_stones && type <= GemCut::square_cut) {
                        model[type - GemCut::gizzard_stones].glRender();
                    } else {
                        model[GemCut::point_cut - GemCut::gizzard_stones].glRender();
                    }
                }   break;
                case ItemType::block: {
                    static Model model("models/block.mdl");
                    model.glRender();
                }   break;
                case gem_rough: {
                    static Model model("models/gem_rough.mdl");
                    model.glRender();
                }   break;
                case stone: {
                    static Model model("models/stone.mdl");
                    model.glRender();
                }   break;
                case wood: {
                    static Model model("models/wood.mdl");
                    model.glRender();
                }   break;
                case door: {
                    static Model model("models/door.mdl");
                    model.glRender();
                }   break;
                case flood_gate: {
                    static Model model("models/flood_gate.mdl");
                    model.glRender();
                }   break;
                case bed: {
                    static Model model("models/bed.mdl");
                    model.glRender();
                }   break;
                case chair: {
                    static Model model("models/chair.mdl");
                    model.glRender();
                }   break;
                case chain: {
                    static Model model("models/chain.mdl");
                    model.glRender();
                }   break;
                case flask: {
                    static Model model("models/flask.mdl");
                    model.glRender();
                }   break;
                case goblet: {
                    static Model model("models/goblet.mdl");
                    model.glRender();
                }   break;
                case window: {
                    static Model model("models/window.mdl");
                    model.glRender();
                }   break;
                case cage: {
                    static Model model("models/cage.mdl");
                    model.glRender();
                }   break;
                case barrel: {
                    static Model model("models/barrel.mdl");
                    model.glRender();
                }   break;
                case bucket: {
                    static Model model("models/bucket.mdl");
                    model.glRender();
                }   break;
                case animal_trap: {
                    static Model model("models/animal_trap.mdl");
                    model.glRender();
                }   break;
                case table: {
                    static Model model("models/table.mdl");
                    model.glRender();
                }   break;
                case coffin: {
                    static Model model("models/coffin.mdl");
                    model.glRender();
                }   break;
                case bin: {
                    static Model model("models/bin.mdl");
                    model.glRender();
                }   break;
                case armor_stand: {
                    static Model model("models/armor_stand.mdl");
                    model.glRender();
                }   break;
                case weapon_rack: {
                    static Model model("models/weapon_rack.mdl");
                    model.glRender();
                }   break;
                case cabinet: {
                    static Model model("models/cabinet.mdl");
                    model.glRender();
                }   break;
                case amulet: {
                    static Model model("models/amulet.mdl");
                    model.glRender();
                }   break;
                case sceptor: {
                    static Model model("models/sceptor.mdl");
                    model.glRender();
                }   break;
                case crown: {
                    static Model model("models/crown.mdl");
                    model.glRender();
                }   break;
                case ring: {
                    static Model model("models/ring.mdl");
                    model.glRender();
                }   break;
                case earring: {
                    static Model model("models/earring.mdl");
                    model.glRender();
                }   break;
                case bracelet: {
                    static Model model("models/bracelet.mdl");
                    model.glRender();
                }   break;
                case gem_large: {
                    static Model model("models/gem_large.mdl");
                    model.glRender();
                }   break;
                case anvil: {
                    static Model model("models/anvil.mdl");
                    model.glRender();
                }   break;
                case meat: {
                    static Model model("models/meat.mdl");
                    model.glRender();
                }   break;
                case fish_prepared: {
                    static Model model("models/fish_prepared.mdl");
                    model.glRender();
                }   break;
                case fish_raw: {
                    static Model model("models/fish_raw.mdl");
                    model.glRender();
                }   break;
                case seed: {
                    static Model model("models/seed.mdl");
                    model.glRender();
                }   break;
                case skin_tanned: {
                    static Model model("models/skin_tanned.mdl");
                    model.glRender();
                }   break;
                case thread: {
                    static Model model("models/thread.mdl");
                    model.glRender();
                }   break;
                case cloth: {
                    static Model model("models/cloth.mdl");
                    model.glRender();
                }   break;
                case totem: {
                    static Model model("models/totem.mdl");
                    model.glRender();
                }   break;
                case backpack: {
                    static Model model("models/backpack.mdl");
                    model.glRender();
                }   break;
                case quiver: {
                    static Model model("models/quiver.mdl");
                    model.glRender();
                }   break;
                case catapult_part: {
                    static Model model("models/catapult_part.mdl");
                    model.glRender();
                }   break;
                case ballista_part: {
                    static Model model("models/ballista_part.mdl");
                    model.glRender();
                }   break;
                case ballista_arrow_head: {
                    static Model model("models/ballista_arrow_head.mdl");
                    model.glRender();
                }   break;
                case trap_part: {
                    static Model model("models/trap_part.mdl");
                    model.glRender();
                }   break;
                case drink: {
                    static Model model("models/drink.mdl");
                    model.glRender();
                }   break;
                case powder: {
                    static Model model("models/powder.mdl");
                    model.glRender();
                }   break;
                case cheese: {
                    static Model model("models/cheese.mdl");
                    model.glRender();
                }   break;
                case liquid: {
                    static Model model("models/liquid.mdl");
                    model.glRender();
                }   break;
                case coin: {
                    static Model model("models/coin.mdl");
                    model.glRender();
                }   break;
                case glob: {
                    static Model model("models/glob.mdl");
                    model.glRender();
                }   break;
                case rock_small: {
                    static Model model("models/rock_small.mdl");
                    model.glRender();
                }   break;
                case pipe_section: {
                    static Model model("models/pipe_section.mdl");
                    model.glRender();
                }   break;
                case hatch_cover: {
                    static Model model("models/hatch_cover.mdl");
                    model.glRender();
                }   break;
                case grate: {
                    static Model model("models/grate.mdl");
                    model.glRender();
                }   break;
                case quern: {
                    static Model model("models/quern.mdl");
                    model.glRender();
                }   break;
                case millstone: {
                    static Model model("models/millstone.mdl");
                    model.glRender();
                }   break;
                case splint: {
                    static Model model("models/splint.mdl");
                    model.glRender();
                }   break;
                case crutch: {
                    static Model model("models/crutch.mdl");
                    model.glRender();
                }   break;
                case traction_bench: {
                    static Model model("models/traction_bench.mdl");
                    model.glRender();
                }   break;
                case cast: {
                    static Model model("models/cast.mdl");
                    model.glRender();
                }   break;
                case slab: {
                    static Model model("models/slab.mdl");
                    model.glRender();
                }   break;
                case egg: {
                    static Model model("models/egg.mdl");
                    model.glRender();
                }   break;
                case book: {
                    static Model model("models/book.mdl");
                    model.glRender();
                }   break;
                default:
                    glCube();
                    break;
                }

                if (reset) {
                    glResetSpecular();
                }

                glPopMatrix();
            }
        }
    }
}

GLvoid glPaintTile(dword tile_x, dword tile_y, dword tile_z, dword zend) {
    TileStructure::Enum s = df.map.tileStructure(tile_x, tile_y, tile_z);
    TileInfo info = df.map.tileInfo(tile_x, tile_y, tile_z);
    TileDefinition def = tileDefinition(s);

    static float constexpr spec_color_0[3] = {0, 0, 0},
                           spec_color_metal[3] = {.3, .3, .3},
                           spec_color_glass[3] = {.1, .1, .1};
    float constexpr spec_angle_metal = M_PI / 8, spec_angle_glass = M_PI / 4;

    bool metal = false, glass = false;

    float color[4] = {.5, .5, .5, 1};
    float & r = color[0], & g = color[1], & b = color[2];

    if (!info.invisible && !def.treetype) {
        switch (def.tilefeaturetype) {
        using namespace TileFeatureType;
        case layer_rock:
        case layer_soil:
        case grass: { // mix layer color with grass color
            int32 species = df.map.getLayerSpecies(tile_x, tile_y, tile_z);
            InorganicRaw & raw = *df.map.inorganic_raw[species];
            float const * rgb;
            if (&raw) {
                rgb = df.checkFineColor(raw.material.state_color.solid, raw.material.tile_color_fore, raw.material.tile_color_fore_bright);
                metal = raw.material.flags->is_metal;
                glass = raw.material.flags->is_glass;
            } else {
                rgb = df.getHardCodedMaterialColor(HardCodeMaterial::rock);
            }
            r = rgb[0];
            g = rgb[1];
            b = rgb[2];
        }   break;
        case mineral: {
            int32 species = df.map.getMineralSpecies(tile_x, tile_y, tile_z);
            InorganicRaw & raw = *df.map.inorganic_raw[species];
            float const * rgb;
            if (&raw) {
                rgb = df.checkFineColor(raw.material.state_color.solid, raw.material.tile_color_fore, raw.material.tile_color_fore_bright);
                metal = raw.material.flags->is_metal;
                glass = raw.material.flags->is_glass;
            } else {
                rgb = df.getHardCodedMaterialColor(HardCodeMaterial::rock);
            }
            if (metal) {
                glMaterialfv(GL_FRONT, GL_SPECULAR, spec_color_metal);
                glMaterialf(GL_FRONT, GL_SHININESS, spec_angle_metal);
            } else if (glass) {
                glMaterialfv(GL_FRONT, GL_SPECULAR, spec_color_glass);
                glMaterialf(GL_FRONT, GL_SHININESS, spec_angle_glass);
            }
            glColor3fv(rgb);
            glCluster(tile_x, tile_y, tile_z, 3, .4);
            if (metal || glass) {
                glMaterialfv(GL_FRONT, GL_SPECULAR, spec_color_0);
                metal = glass = false;
            }
            if (&raw) {
                rgb = df.checkFineColor(raw.material.state_color.solid, raw.material.tile_color_back, 0);
            } else {
                rgb = df.getHardCodedMaterialColor(HardCodeMaterial::rock);
            }
            r = rgb[0];
            g = rgb[1];
            b = rgb[2];
        }   break;
        case obsidian: {
            int32 species = df.map.getLavaStoneSpecies(tile_x, tile_y, tile_z);
            InorganicRaw & raw = *df.map.inorganic_raw[species];
            float const * rgb;
            if (&raw) {
                rgb = df.checkFineColor(raw.material.state_color.solid, raw.material.tile_color_fore, raw.material.tile_color_fore_bright);
                metal = raw.material.flags->is_metal;
                glass = raw.material.flags->is_glass;
            } else {
                rgb = df.getHardCodedMaterialColor(HardCodeMaterial::rock);
            }
            r = rgb[0];
            g = rgb[1];
            b = rgb[2];
//            r = .13;
//            g = .20;
//            b = .15;
//            shiny = true; // volcanic glass
//            spec_r = spec_g = spec_b = .1;
//            spec_angle = M_PI / 4;
        }   break;
        case ice:
            r = .87;
            g = .98;
            b = .98;
            glass = true;
            break;
        case none:
            switch (s) {
            using namespace TileStructure;
            case semi_molten_rock:
                r = .81;
                g = .06;
                b = .13;
                //todo: emits_light = true
                break;
            default:
                break;
            }
            break;
        default:
            float const * rgb = df.getHardCodedMaterialColor(HardCodeMaterial::rock);
            r = rgb[0];
            g = rgb[1];
            b = rgb[2];
            break;
        }
        if (def.tilefeaturetype == TileFeatureType::grass) { // grass color bending. todo genuine grass model
            byte grass_density;
            int32 grass_species;
            df.map.getGrass(tile_x, tile_y, tile_z, grass_density, grass_species);
            VegetationRaw & grass = *df.map.vegetation_raw[grass_species];
            float density = grass_density / 100.;
            byte console_color, console_bright;
            float const * grass_rgb;
            if (&grass) {
                switch (def.grasstype) {
                using namespace GrassType;
                case normal_1:
                default:
                    console_color = grass.grass_color_1_fore;
                    console_bright = grass.grass_color_1_fore_bright;
                    break;
                case normal_2:
                    console_color = grass.grass_color_2_fore;
                    console_bright = grass.grass_color_2_fore_bright;
                    break;
                case dry:
                    console_color = grass.grass_color_dry_fore;
                    console_bright = grass.grass_color_dry_fore_bright;
                    break;
                case dead:
                    console_color = grass.grass_color_dead_fore;
                    console_bright = grass.grass_color_dead_fore_bright;
                    break;
                }
                if (grass_density) {
                    metal = df.isMetal(grass.basic_material, grass.basic_species);
                    glass = df.isGlass(grass.basic_material, grass.basic_species);
                }
                grass_rgb = df.getColorOfMaterial(grass.basic_material, grass.basic_species, MaterialState::solid, console_color, console_bright, true);
            } else {
                switch (def.grasstype) {
                using namespace GrassType;
                case normal_1:
                default:
                    console_color = 2;
                    console_bright = 0;
                    break;
                case normal_2:
                    console_color = 2;
                    console_bright = 1;
                    break;
                case dry:
                    console_color = 6;
                    console_bright = 1;
                    break;
                case dead:
                    console_color = 6;
                    console_bright = 0;
                    break;
                }
                grass_rgb = df.getConsoleColor(console_color, console_bright);
            }
            r = r * (1 - density) + grass_rgb[0] * density;
            g = g * (1 - density) + grass_rgb[1] * density;
            b = b * (1 - density) + grass_rgb[2] * density;
        }

        if (!info.light) {
            r *= .75;
            g *= .75;
            b *= .75;
        } // todo lower specular reflection

        glColor4fv(color);
        if (metal) {
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec_color_metal);
            glMaterialf(GL_FRONT, GL_SHININESS, spec_angle_metal);
        } else if (glass) {
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec_color_glass);
            glMaterialf(GL_FRONT, GL_SHININESS, spec_angle_glass);
        }

        if (def.hasfloor) {
            glFloor(tile_x, tile_y, tile_z);
            glPushMatrix();
            glTranslatef(0, 0, floor_height);
        }
        switch (def.bodytype) {
        case BodyType::wall:
            glWall(tile_x, tile_y, tile_z);
            break;
        case BodyType::fortification:
//            glFortification();
            break;
        case BodyType::rampup_trivial:
            glRampUp(tile_x, tile_y, tile_z);
            break;
        case BodyType::rampdown:
            break;
        case BodyType::stairboth:
            glStairBoth(tile_x, tile_y, tile_z);
            break;
        case BodyType::stairdown:
            glStairDown(tile_x, tile_y, tile_z);
            break;
        case BodyType::stairup:
            glStairUp();
            break;
        case BodyType::track:
//            glTrack();
            break;
        case BodyType::trackup:
//            glTrackUp();
            break;
        case BodyType::pebble:
            glPebble(tile_x, tile_y, tile_z);
            break;
        case BodyType::boulder:
            glBoulder(tile_x, tile_y, tile_z);
            break;
        }
        if (def.hasfloor) {
            glPopMatrix();
        }
        if (metal || glass) {
            glMaterialfv(GL_FRONT, GL_SPECULAR, spec_color_0);
        }
    } else if (info.invisible) { // invisible
        if (tile_z == zend || !df.map.tileInfo(tile_x, tile_y, tile_z + 1).invisible) {
            glColor3f(0, 0, 0);
            glBegin(GL_QUADS);
            glVertex3f(-.5, -.5,   1);
            glVertex3f(-.5,  .5,   1);
            glVertex3f( .5,  .5,   1);
            glVertex3f( .5, -.5,   1);
            glEnd();
        }
//        glColor3f(0, 0, 0);
//        glCube();
    }
    switch (info.designation) {
    using namespace Designation;
    case none:
        break;
    case generic_remove:
        glColor3f(.5, .5, .5);
        glCursor();
        break;
    case stair_both:
        break;
    case channel:
        break;
    case ramp_up:
        break;
    case stair_down:
        break;
    case stair_up:
        break;
    }
}

GLvoid glLiquid(int tile_x, int tile_y, int tile_z) {
    float constexpr unit_liquid = 1. / 8.;
    int8 liquid_level[3][3];
    for (dword x = 0; x < 3; ++x) {
        for (dword y = 0; y < 3; ++y) {
            liquid_level[x][y] = df.map.tileInfo(tile_x + x - 1, tile_y + y - 1, tile_z).liquid_level;
            if (liquid_level[x][y] == 0 && df.map.tileDefinition(tile_x + x - 1, tile_y + y - 1, tile_z).isblocked) {
                liquid_level[x][y] = -1;
            }
        }
    }
    bool liquid = false, wall = false; // unblocked liquid, or liquid in wall chamfer
    bool cornerliquid[4] = {false, false, false, false};
    TileDefinition center = df.map.tileDefinition(tile_x, tile_y, tile_z);
    if (liquid_level[1][1] > 0) {
        liquid = true;
    } else {
        if (center.bodytype == BodyType::wall) {
            bool conn[4];
            if (center.hasconnection) {
                conn[0] = center.connection.east;
                conn[1] = center.connection.north;
                conn[2] = center.connection.west;
                conn[3] = center.connection.south;
            } else {
                TileDefinition s[4];
                s[0] = df.map.tileDefinition(tile_x + 1, tile_y    , tile_z);
                s[1] = df.map.tileDefinition(tile_x    , tile_y - 1, tile_z);
                s[2] = df.map.tileDefinition(tile_x - 1, tile_y    , tile_z);
                s[3] = df.map.tileDefinition(tile_x    , tile_y + 1, tile_z);
                for (int i = 0; i < 4; i++) {
                    conn[i] = s[i].isblocked || (!s[i].treetype && s[i].isupslope);
                }
            }
            bool corner[4];
            for (dword i = 0; i < 4; ++i) {
                corner[i] = conn[i] | conn[(i + 1) & 3];
            }
            cornerliquid[0] = !corner[0] && (liquid_level[2][1] > 0 || liquid_level[2][0] > 0 || liquid_level[1][0] > 0);
            cornerliquid[1] = !corner[1] && (liquid_level[0][1] > 0 || liquid_level[0][0] > 0 || liquid_level[1][0] > 0);
            cornerliquid[2] = !corner[2] && (liquid_level[0][1] > 0 || liquid_level[0][2] > 0 || liquid_level[1][2] > 0);
            cornerliquid[3] = !corner[3] && (liquid_level[2][1] > 0 || liquid_level[2][2] > 0 || liquid_level[1][2] > 0);
            for (dword i = 0; i < 4; ++i) {
                wall |= cornerliquid[i];
            }
        }
    }
    if (liquid || wall) {
        float grid[5][5];
        for (dword x = 0; x < 3; ++x) {
            for (dword y = 0; y < 3; ++y) {
                if (liquid_level[x][y] > 0) {
                    grid[x * 2][y * 2] = liquid_level[x][y] * unit_liquid;
                } else {
                    grid[x * 2][y * 2] = 0;
                }
                // ramps
//                TileDefinition center = df.map.tileDefinition(tile_x + x - 1, tile_y + y - 1, tile_z);
//                if (center.isupslope) {
//                    dword tree = center.treetype;
//                    bool up = false;
//                    for (dword a = 0; !up && a < 3; ++a) {
//                        for (dword b = 0; !up && b < 3; ++b) {
//                            if (a != 1 || b != 1) {
//                                TileDefinition def = df.map.tileDefinition(tile_x + x + a - 2, tile_y + y + b - 2, tile_z);
//                                if (def.isupslope && !(tree ^ def.treetype)) {
//                                    up = true;
//                                }
//                                TileInfo above_info = df.map.tileInfo(tile_x + x + a - 2, tile_y + y + b - 2, tile_z + 1);
//                                if (def.isblocked && above_info.liquid_level && (!tree || def.treetype)) {
//                                    up = true;
//                                }
//                            }
//                        }
//                    }
//                    if (up) {
//                        grid[x * 2][y * 2] = grid[x * 2][y * 2] * .5 + .5;
//                    }
//                }
                if (df.map.tileDefinition(tile_x + x - 1, tile_y + y - 1, tile_z).isupslope) {
                    grid[x * 2][y * 2] = grid[x * 2][y * 2] * .5 + .5;
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
                // ramps
                dword tree_slope = 0, trivial_slope = 0, tree_up = 0, trivial_up = 0;
                for (dword a = 0; a < 2; ++a) {
                    for (dword b = 0; b < 2; ++b) {
                        TileDefinition def = df.map.tileDefinition(tile_x + x + a - 1, tile_y + y + b - 1, tile_z);
                        if (def.isupslope) {
                            if (def.treetype) {
                                ++tree_slope;
                            } else {
                                ++trivial_slope;
                            }
                        }
                        TileInfo above_info = df.map.tileInfo(tile_x + x + a - 1, tile_y + y + b - 1, tile_z + 1);
                        if (def.isblocked && above_info.liquid_level) {
                            ++trivial_up;
                            if (def.treetype) {
                                ++tree_up;
                            }
                        }
                    }
                }
                if ((tree_slope && tree_up) || (trivial_slope && trivial_up)) {
                    grid[x * 2 + 1][y * 2 + 1] = 1;
                } else if (tree_slope == 4 || trivial_slope == 4) {
                    grid[x * 2 + 1][y * 2 + 1] = grid[x * 2 + 1][y * 2 + 1] * .5 + .5;
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
                // ramps
                dword tree_slope = 0, trivial_slope = 0, tree_up = 0, trivial_up = 0;
                for (dword a = 0; a < 2; ++a) {
                    TileDefinition def = df.map.tileDefinition(tile_x + x + a - 1, tile_y + y - 1, tile_z);
                    if (def.isupslope) {
                        if (def.treetype) {
                            ++tree_slope;
                        } else {
                            ++trivial_slope;
                        }
                    }
                    TileInfo above_info = df.map.tileInfo(tile_x + x + a - 1, tile_y + y - 1, tile_z + 1);
                    if (def.isblocked && above_info.liquid_level) {
                        ++trivial_up;
                        if (def.treetype) {
                            ++tree_up;
                        }
                    }
                }
                if ((tree_slope && tree_up) || (trivial_slope && trivial_up)) {
                    grid[x * 2 + 1][y * 2] = 1;
                } else if (tree_slope == 2 || trivial_slope == 2) {
                    grid[x * 2 + 1][y * 2] = grid[x * 2 + 1][y * 2] * .5 + .5;
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
                // ramps
                dword tree_slope = 0, trivial_slope = 0, tree_up = 0, trivial_up = 0;
                for (dword b = 0; b < 2; ++b) {
                    TileDefinition def = df.map.tileDefinition(tile_x + x - 1, tile_y + y + b - 1, tile_z);
                    if (def.isupslope) {
                        if (def.treetype) {
                            ++tree_slope;
                        } else {
                            ++trivial_slope;
                        }
                    }
                    TileInfo above_info = df.map.tileInfo(tile_x + x - 1, tile_y + y + b - 1, tile_z + 1);
                    if (def.isblocked && above_info.liquid_level) {
                        ++trivial_up;
                        if (def.treetype) {
                            ++tree_up;
                        }
                    }
                }
                if ((tree_slope && tree_up) || (trivial_slope && trivial_up)) {
                    grid[x * 2][y * 2 + 1] = 1;
                } else if (tree_slope == 2 || trivial_slope == 2) {
                    grid[x * 2][y * 2 + 1] = grid[x * 2][y * 2 + 1] * .5 + .5;
                }
            }
        }
        vector3f normals[3][3];
        if (liquid) {
            normals[1][1] = vector3f(
                grid[1][1] / 2 + grid[1][2] + grid[1][3] / 2 - grid[3][1] / 2 - grid[3][2] - grid[3][3] / 2,
                grid[1][1] / 2 + grid[2][1] + grid[3][1] / 2 - grid[1][3] / 2 - grid[2][3] - grid[3][3] / 2,
                2).normalize();
        }
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
        static float constexpr color0[] = {0, 0, 0}, specular[] = {.3, .3, .3}, emission[] = {.37, .07, .00};
        TileInfo info = df.map.tileInfo(tile_x, tile_y, tile_z);
        if (info.lava) {
            glMaterialfv(GL_FRONT, GL_EMISSION, emission);
            glColor4f(.74, .15, .00, .95);
        } else {
            glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
            glMaterialf(GL_FRONT, GL_SHININESS, M_PI / 8);
            if (info.stagnant) {
                if (info.salty) { // stagnant salty water
                    glColor4f(.26, .32, .33, .80);
                } else { // stagnant water
                    glColor4f(.26, .33, .29, .70);
                }
            } else if (info.salty) { // salty water
                glColor4f(.47, .84, .80, .30);
            } else { // fresh water
                glColor4f(.63, .93, .91, .20);
            }
        }
        if (liquid) {
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
        } else {
            float constexpr cut = .293;
            float constexpr remain = .5 - cut;
            float constexpr cut2 = cut * 2;
            float constexpr remain2 = remain * 2;
            glBegin(GL_TRIANGLES);
            if (cornerliquid[0]) {
                glNormal3fv(normals[2][0]);
                glVertex3f(.5, -.5, grid[3][1]);
                glNormal3fv(normals[2][0] * remain2 + normals[1][0] * cut2);
                glVertex3f(remain, -.5, grid[3][1] + (grid[2][1] - grid[3][1]) * cut2);
                glNormal3fv(normals[2][0] * remain2 + normals[2][1] * cut2);
                glVertex3f(.5, -remain, grid[3][1] + (grid[3][2] - grid[3][1]) * cut2);
            }
            if (cornerliquid[1]) {
                glNormal3fv(normals[0][0]);
                glVertex3f(-.5, -.5, grid[1][1]);
                glNormal3fv(normals[0][0] * remain2 + normals[0][1] * cut2);
                glVertex3f(-.5, -remain, grid[1][1] + (grid[1][2] - grid[1][1]) * cut2);
                glNormal3fv(normals[0][0] * remain2 + normals[1][0] * cut2);
                glVertex3f(-remain, -.5, grid[1][1] + (grid[2][1] - grid[1][1]) * cut2);
            }
            if (cornerliquid[2]) {
                glNormal3fv(normals[0][2]);
                glVertex3f(-.5, .5, grid[1][3]);
                glNormal3fv(normals[0][2] * remain2 + normals[1][2] * cut2);
                glVertex3f(-remain, .5, grid[1][3] + (grid[2][3] - grid[1][3]) * cut2);
                glNormal3fv(normals[0][2] * remain2 + normals[0][1] * cut2);
                glVertex3f(-.5, remain, grid[1][3] + (grid[1][2] - grid[1][3]) * cut2);
            }
            if (cornerliquid[3]) {
                glNormal3fv(normals[2][2]);
                glVertex3f(.5, .5, grid[3][3]);
                glNormal3fv(normals[2][2] * remain2 + normals[2][1] * cut2);
                glVertex3f(.5, remain, grid[3][3] + (grid[3][2] - grid[3][3]) * cut2);
                glNormal3fv(normals[2][2] * remain2 + normals[1][2] * cut2);
                glVertex3f(remain, .5, grid[3][3] + (grid[2][3] - grid[3][3]) * cut2);
            }
            glEnd();
        }
        if (info.lava) {
            glMaterialfv(GL_FRONT, GL_EMISSION, color0);
        } else {
            glMaterialfv(GL_FRONT, GL_SPECULAR, color0);
        }
    }
}

float constexpr viewport_size = .293;
float scrPorpotion = 1;
double matrix_frustum[16], matrix_ortho[16], matrix_frustum_model[16]; // Unprojection requires double
GLint viewport[4];

void glResize(GLsizei width, GLsizei height) {
    glViewport(0, 0, width, height);
    glGetIntegerv(GL_VIEWPORT, viewport);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    scrPorpotion = (width == 0 || height == 0) ? 1 : (float) width / (float) height;
    float w, h, d;
    if (width <= height) {
        w = viewport_size;
        h = w / scrPorpotion;
        d = w * 1.5;
        float _d = d / (5. / 128);
        w /= _d;
        h /= _d;
        d /= _d;
        glFrustum(-w, w, -h, h, d, 5);
        glTranslatef(0, 0, -d);
    } else {
        h = viewport_size;
        w = h * scrPorpotion;
        d = h * 1.5;
        float _d = d / (5. / 128);
        w /= _d;
        h /= _d;
        d /= _d;
        glFrustum(-w, w, -h, h, d, 5);
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

TEXT_SIZE glText(String const & s) {
    return glText(s.data(), s.length());
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

inline void glQuadText(String const & s, float x, float y, float height, text_align align) {
    glQuadText(s.data(), s.length(), x, y, height, align);
}

inline void glQuadText(string const & s, float x, float y, float height, text_align align) {
    glQuadText(s.data(), s.length(), x, y, height, align);
}

float center_x = 0, center_y = 0, center_z = 0;
float constexpr tile_size = .1;
int32 const view_radius = 40, view_depth = 10;
float constexpr eye_distance = 1;
float view_angle = M_PI / 4 * 3, pitch_angle = M_PI / 4;

bool hud_repaint = true;
void hudDraw();

struct static_string {
    char const * text;
    int len;
};

bool glRepaint() {
    clock_t time = clock();

    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

//    Leave it for the exe to handle
//    DWORD status = -1;
//    if (!dfModule || !GetExitCodeProcess(__todo_get_process, &status) || status != STILL_ACTIVE) { // process finished
//        cout << "dwarf escaped!" << endl;
//        PostQuitMessage(status);
//    }
    if (df.cursor.x != invalid_location) { // todo
        center_x = limit(df.cursor.x - view_radius * .5f, center_x, df.cursor.x + view_radius * .5f);
        center_y = limit(df.cursor.y - view_radius * .5f, center_y, df.cursor.y + view_radius * .5f);
        center_z = df.cursor.z;
    }

    float height = 1. / df.view_height;
    UI::Base & ui0 = df.getTopmostUI();
    switch (ui0.getUItype()) {
    case UI::main_menu: {
        UI::MainMenu & ui = *(UI::MainMenu *) &ui0;
        glPushAttrib(GL_ENABLE_BIT);
        glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(matrix_ortho);
        glMatrixMode(GL_MODELVIEW);

        static static_string constexpr strings[7] = {
            "Continue Playing", 16,
            "Start Playing", 13,
            "Create New World!", 17,
            "Design New World with Advanced Parameters", 41,
            "Object Testing Arena", 20,
            "About DF", 8,
            "Quit", 4
        };

        glColor3fv(df.getConsoleColor(ConsoleColor::light_gray, 0));
        glQuadText(ui.version, scrPorpotion, 1 - height, height, text_align::right);
        glColor3fv(df.getConsoleColor(ConsoleColor::light_cyan, 0));
        glQuadText(ui.credit_1, scrPorpotion * .25, 1 - height * 3, height, text_align::center);
        glQuadText(ui.credit_2, scrPorpotion * .25, 1 - height * 2, height, text_align::center);
        glQuadText("DorfyVision by Qrox", scrPorpotion * .25, 1 - height    , height, text_align::center);
        glQuadText(ui.website_1, scrPorpotion * .75, 1 - height * 3, height, text_align::center);
        glQuadText(ui.website_2, scrPorpotion * .75, 1 - height * 2, height, text_align::center);
        glQuadText("DorfyVision Contact: Qrox@sina.com", scrPorpotion * .75, 1 - height * 1, height, text_align::center);
        glTranslatef(scrPorpotion / 2, 0, 0);
        glColor3fv(df.getConsoleColor(ConsoleColor::light_red, 0));
        glQuadText(ui.title, 0, height * 3, height, text_align::center);
        glColor3fv(df.getConsoleColor(ConsoleColor::white, 0));
        glQuadText(ui.subtitle, 0, height * 4, height, text_align::center);
        glColor3fv(df.getConsoleColor(ConsoleColor::light_gray, 0));
        glQuadText(ui.splash_text, strlen(ui.splash_text), 0, height * 6, height, text_align::center);
        glTranslatef(0, .5 - height * ui.entries.size() / 2, 0);
        dword count = 0;
        for (auto curr = ui.entries.begin(), end = ui.entries.end(); curr < end; ++curr, ++count) {
            int index = *curr;
            if (ui.selection == count) {
                glColor3fv(df.getConsoleColor(ConsoleColor::white, 0));
            } else {
                glColor3fv(df.getConsoleColor(ConsoleColor::dark_gray, 0));
            }
            glQuadText(strings[index].text, strings[index].len, 0, 0, height, text_align::center);
            glTranslatef(0, height, 0);
        }
        glPopAttrib();
    }   break;
    case UI::loading: {
        UI::Loading & ui = *(UI::Loading *) &ui0;
        glPushAttrib(GL_ENABLE_BIT);
        glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(matrix_ortho);
        glMatrixMode(GL_MODELVIEW);

        dword entries_per_screen = (df.view_height - 4) / 2;
        dword selection = ui.selection;
        dword cursorat = selection % entries_per_screen;
        dword start = selection - cursorat;

        glColor3fv(df.getConsoleColor(ConsoleColor::white, 0));
        glQuadText("Dwarf Fortress Options: Load Game", scrPorpotion / 2, 0, height, text_align::center);

        glTranslatef(0, height * 2, 0);
        auto curr = ui.save_info.begin() + start,
             end  = curr + entries_per_screen;
        if (end > ui.save_info.end()) {
            end = ui.save_info.end();
        }
        stringstream s;
        for (dword index = start; curr < end; ++curr, ++index) {
            UI::Loading::SaveInfo & saveinfo = **curr;
            if (&saveinfo) {
                if (index == selection) {
                    glColor3fv(df.getConsoleColor(ConsoleColor::white, 0));
                } else {
                    glColor3fv(df.getConsoleColor(ConsoleColor::light_gray, 0));
                }
                s.str(""); s.clear();
                s << saveinfo.fortress_name << ", " << saveinfo.saveTypeString();
                glQuadText(s.str(), height * 2, 0, height, text_align::left);
                s.str(""); s.clear();
                s << "Folder: " << saveinfo.folder_name;
                glQuadText(s.str(), height * 3, height, height, text_align::left);
                s.str(""); s.clear();
                s << saveinfo.world_name << ", " << saveinfo.year;
                glQuadText(s.str(), scrPorpotion * .625, 0, height, text_align::left);
                s.str(""); s.clear();
            }
            glTranslatef(0, height * 2, 0);
        }
        glPopAttrib();
    }   break;
    case UI::options: {
        UI::Options & ui = *(UI::Options *) &ui0;
        glPushAttrib(GL_ENABLE_BIT);
        glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(matrix_ortho);
        glMatrixMode(GL_MODELVIEW);

        static static_string constexpr strings[7] = {
            "Return to Game", 14,
            "Save Game", 9,
            "Key Bindings", 12,
            "Export Local Image", 18,
            "Music and Sound", 15,
            "Retire the Fortress (for the time being)", 40,
            "Abandon the Fortress to Ruin", 28,
        };

        glTranslatef(scrPorpotion / 2, .5 - height * ui.entries.size() / 2, 0);
        DWORD count = 0;
        for (auto curr = ui.entries.begin(), end = ui.entries.end(); curr < end; ++curr, ++count) {
            int index = *curr;
            if (ui.selection == count) {
                glColor3fv(df.getConsoleColor(ConsoleColor::white, 0));
            } else {
                glColor3fv(df.getConsoleColor(ConsoleColor::dark_gray, 0));
            }
            glQuadText(strings[index].text, strings[index].len, 0, 0, height, text_align::center);
            glTranslatef(0, height, 0);
        }
        glPopAttrib();
    }   break;
    case UI::announcement: {
        UI::Announcement & ui = *(UI::Announcement *) &ui0;
        glPushAttrib(GL_ENABLE_BIT);
        glEnable(GL_TEXTURE_2D);
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(matrix_ortho);
        glMatrixMode(GL_MODELVIEW);

        if (!ui.announcements.empty()) {
            dword msg_per_screen = df.view_height - 5;
            dword selection = ui.selection;
            dword cursorat = selection % msg_per_screen;
            dword start = selection - cursorat;
            UI::Announcement::Data & cursor = *ui.announcements[selection];
            glColor3fv(df.getConsoleColor(ConsoleColor::light_green, 0));
            glQuadText("z", height * 2, 1 - height * 2, height, text_align::left);
            if (&cursor) {
                glQuadText("¡ú", height * 1, height * (cursorat + 2), height, text_align::left);
            }
            if (&cursor && cursor.location.x != invalid_location) {
                glColor3fv(df.getConsoleColor(ConsoleColor::white, 0));
            } else {
                glColor3fv(df.getConsoleColor(ConsoleColor::dark_gray, 0));
            }
            glQuadText(": Zoom to location", height * 3, 1 - height * 2, height, text_align::left);
            glTranslatef(0, height * 2, 0);
            auto curr = ui.announcements.begin() + start,
                 end  = curr + msg_per_screen;
            if (end > ui.announcements.end()) {
                end = ui.announcements.end();
            }
            for (; curr < end; ++curr) {
                UI::Announcement::Data & announcement = **curr;
                if (&announcement) {
                    glColor3fv(df.getConsoleColor(announcement.color, announcement.bright));
                    glQuadText(announcement.text, height * 2, 0, height, text_align::left);
                    stringstream s;
                    if (announcement.repeat) {
                        s << "x" << announcement.repeat + 1;
                        glColor3f(1, 1, 1);
                        glQuadText(s.str(), scrPorpotion - height * 2, 0, height, text_align::right);
                    }
                }
                glTranslatef(0, height, 0);
            }
        } else {
            glColor3f(1, 1, 1);
            glQuadText("No recent announcements.", height * 2, height * 2, height, text_align::left);
        }
        glPopAttrib();
    }   break;
    case UI::dwarf_fortress: {
        glClearColor(.5, .5, .5, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        static bool cursor_initialized = false;
        if (!cursor_initialized) {
            if (df.cursor.x != invalid_location) {
                center_x = df.cursor.x;
                center_y = df.cursor.y;
                center_z = df.cursor.z;
            } else {
                center_x = df.view_x + df.getFortressModeViewWidth()  / 2.;
                center_y = df.view_y + df.getFortressModeViewHeight() / 2.;
                center_z = df.view_z;
            }
            cursor_initialized = true;
        }

        glPushAttrib(GL_ENABLE_BIT);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

        shader_prog.apply();
        shader_prog.passUniform4f(shader_fog, .5, .5, .5, 1);
        shader_prog.passUniform1f(shader_start, tile_size * view_radius * .5);
        shader_prog.passUniform1f(shader_k, -10);
//        glEnable(GL_FOG);

        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(matrix_frustum);
        glMatrixMode(GL_MODELVIEW);
        gluLookAt(eye_distance * cos(pitch_angle) * sin(view_angle),
                  eye_distance * cos(pitch_angle) * cos(view_angle),
                  eye_distance * sin(pitch_angle),
                  0, 0, 0,
                  0, 0,
                  eye_distance * cos(pitch_angle) * cos(pitch_angle) / sin(pitch_angle));
        static float constexpr lefthanded[] = {
             0, 1, 0, 0,
             1, 0, 0, 0,
             0, 0, 1, 0,
             0, 0, 0, 1,
        };
        glMultMatrixf(lefthanded); // flip x and y axis to make the space left-handed, to fit DF's space handedness

        float32 constexpr ang_axis = 0.409; // angle between rotation axis and revolution axis (assume earth, 23¡ã26¡ä)
        float32 constexpr ang_lat = 0.535; // latitude (assume 30.67¡ã)
        float32 constexpr r_rot = 6371393; // rotation radius (assume earth, 6371.393km)
        float32 constexpr r_rev = 149597870700; // revolution radius (assume sun-earth, 149597870700m, 2012.9 IAU)
        float32 constexpr h_atm = 55000; // height of planet atmosphere (assume earth, until ionosphere)

        dword constexpr winter_solstice_month = 12; // Dec
        dword constexpr winter_solstice_day =   22; // 22nd
        int32 constexpr winter_solstice_tick = (winter_solstice_month - tick_0_month) * ticks_per_month + (winter_solstice_day - 1) * ticks_per_day;
        float32 ang_planet_rev = (float32) ((int32) df.ticks - winter_solstice_tick) / ticks_per_year * (M_PI * 2); // angle of revolution, winter solstice ¡Ö dec.22
        float32 ang_planet_rot = (float32) (df.ticks % ticks_per_day) / ticks_per_day * (M_PI * 2); // angle of rotation, midnight = 0, dawn = ¦Ð/2, noon = ¦Ð, dusk = 3¦Ð/2
        float32 ang_planet_axis_term = asin(sin(ang_axis) * cos(ang_planet_rev)); // angle between rotation axis and terminator
        // in planet's equator coordinate (equator in x-y plane, axis to z direction, north pole to +z)
        vector3f planet_axis(0, 0, 1); // planet's rotation axis
        vector3f sol_dir(cos(ang_planet_axis_term), 0, -sin(ang_planet_axis_term)); // direction of the sun at noon from planet center
        sol_dir = sol_dir.rotate(planet_axis * (M_PI - ang_planet_rot)); // direction of the sun from planet center
        vector3f mapz(cos(ang_lat), 0, sin(ang_lat)); // dwarf fortress map up
        vector3f mapy(sin(ang_lat), 0, -cos(ang_lat)); // dwarf fortress map south
        vector3f mapx(0, 1, 0); // dwarf fortress map east
        vector3f mapsol(sol_dir.dot(mapx), sol_dir.dot(mapy), sol_dir.dot(mapz) - r_rot / r_rev); // direction of the sun in dwarf fortress map coordinate
        float32 dip_sol = asin(mapsol[2]); // sun's dip angle to the map
        // atmosphere light attenuation, assume uniform atmosphere and exponential light attenuation
        float32 l_solar_light = (sqrt(sqr(r_rot + h_atm) - sqr(r_rot * cos(dip_sol))) - r_rot * sin(dip_sol)) / h_atm - 1; // solar light's travel distance in the atmosphere / atmosphere height - 1
        float32 constexpr att_r = -.2, att_g = -.3, att_b = -.3; // blue light scatters more quickly
        vector3f solar_light_amb, solar_light_dif, solar_light_spe;
        solar_light_spe = solar_light_dif = vector3f(   // unscattered light until l_solar_light distance
            exp(att_r * l_solar_light),
            exp(att_g * l_solar_light),
            exp(att_b * l_solar_light)
        );
        solar_light_amb = vector3f(                     // scattered light at l_solar_light distance
            -att_r * exp(att_r * l_solar_light),
            -att_g * exp(att_g * l_solar_light),
            -att_b * exp(att_b * l_solar_light)
        );
        GLfloat solar_light_pos[4] = {mapsol[0], mapsol[1], mapsol[2], 0};
        glLightfv(GL_LIGHT0, GL_POSITION, solar_light_pos);
        glLightfv(GL_LIGHT0, GL_AMBIENT, solar_light_amb);
        glLightfv(GL_LIGHT0, GL_DIFFUSE, solar_light_dif);
        glLightfv(GL_LIGHT0, GL_SPECULAR, solar_light_spe);

        // moon, placeholder
        float32 constexpr ang_moon_axis = 0.409; // angle between moon's revolution axis and planet's rotation axis (assume earth's moon, 18.28¡ã~28.58¡ã, use middle value)
        float32 constexpr r_moon_rev = 384401000; // radius of moon's revolution orbit (assume earth's moon, 384401km)
        float32 ang_moon_rev = df.ticks % ticks_per_month / ticks_per_month * (M_PI * 2);
        // in planet's equator coordinate
        vector3f moon_rev(-sin(ang_moon_axis), 0, cos(ang_moon_axis)); // moon's revolution axis
        vector3f planet_rot(0, 0, 1); // planet's rotation axis
        vector3f moon_dir(cos(ang_moon_axis), 0, sin(ang_moon_axis)); // moon's direction from planet center at each months' first midnight
        moon_dir = moon_dir.rotate(moon_rev * ang_moon_rev).rotate(planet_rot * -ang_planet_rot); // moon's current direction from planet center
        vector3f mapmoon(moon_dir.dot(mapx), moon_dir.dot(mapy), moon_dir.dot(mapz) - r_rot / r_moon_rev); // moon's direction in dwarf fortress map coordinate
        float32 dip_moon = asin(mapmoon[2]); // moon's dip angle to the map
        float32 l_moon_light = (sqrt(sqr(r_rot + h_atm) - sqr(r_rot * cos(dip_moon))) - r_rot * sin(dip_moon)) / h_atm - 1; // moon light's travel distance in atmosphere / atmosphere height - 1
        vector3f moon_light_amb, moon_light_dif, moon_light_spe;
        moon_light_spe = moon_light_dif = vector3f(     // unscattered light until l_moon_light distance
            exp(att_r * l_moon_light),
            exp(att_g * l_moon_light),
            exp(att_b * l_moon_light) * 1.3
        ) * .2;
        moon_light_amb = vector3f(                      // scattered light at l_moon_light distance
            -att_r * exp(att_r * l_moon_light),
            -att_g * exp(att_g * l_moon_light),
            -att_b * exp(att_b * l_moon_light) * 1.3    // add some blue to the night
        ) * .2;
        GLfloat moon_light_pos[4] = {mapmoon[0], mapmoon[1], mapmoon[2], 0};
        glLightfv(GL_LIGHT1, GL_POSITION, moon_light_pos);
        glLightfv(GL_LIGHT1, GL_AMBIENT, moon_light_amb);
        glLightfv(GL_LIGHT1, GL_DIFFUSE, moon_light_dif);
        glLightfv(GL_LIGHT1, GL_SPECULAR, moon_light_spe);

//        static float const fog_color[3] = {.5, .5, .5};
//        glFogi(GL_FOG_MODE, GL_EXP2);
//        glFogf(GL_FOG_START, 0.4);
//        glFogf(GL_FOG_END, 0.5);
//        glFogf(GL_FOG_DENSITY, 2.0);
//        glFogfv(GL_FOG_COLOR, fog_color);

        dword z_end = (dword) center_z;
        dword z_start = max(0u, (dword) center_z - view_depth + 1);

        glScalef(tile_size, tile_size, tile_size); // scale
        glTranslatef(-center_x, -center_y, -center_z); // center
        glGetDoublev(GL_MODELVIEW_MATRIX, matrix_frustum_model);

        renderrange range(matrix_frustum_model, matrix_frustum, viewport, view_radius);
        if (range.ok()) {
            // render blocks
            for (auto z = range.begin(df.map.dimension.x - 1, df.map.dimension.y - 1, z_end, 1); z >= z_start; --z) {
                for (auto x = z.begin(1); x <= z.end(); ++x) {
                    for (auto y = x.begin(); y <= x.end(); ++y) {
                        auto tx = y.X(), ty = y.Y(), tz = y.Z();
                        glPushMatrix();
                        glTranslatef(tx, ty, tz);
                        glPaintTile(tx, ty, tz, z_end);
                        glPopMatrix();
                    }
                }
                for (auto x = z.begin(16); x <= z.end(); ++x) {
                    for (auto y = x.begin(); y <= x.end(); ++y) {
                        Block3d & block = df.map.getBlock3d(y.X(), y.Y(), y.Z());
                        if (&block) {
                            glPaintBlock3d(block);
                        }
                    }
                }
            }

            // render vegetation
            auto regions = range.begin(df.map.dimension.x - 1, df.map.dimension.y - 1, z_end, view_depth);
            for (auto x = regions.begin(48); x <= regions.end(); ++x) {
                for (auto y = x.begin(); y <= x.end(); ++y) {
                    Block2d & block = df.map.getBlock2d(y.X(), y.Y());
                    if (&block) {
                        glPaintBlock2d(block, regions);
                    }
                }
            }

            // render creatures
            glPushMatrix();
            glTranslatef(0, 0, floor_height);
            for (auto curr = df.map.creatures.begin(), end = df.map.creatures.end(); curr < end; ++curr) {
                Creature & creature = **curr;
                if (!df.map.tileInfo(creature.location.x, creature.location.y, creature.location.z).invisible &&
                        regions.contains(creature.location.x, creature.location.y, creature.location.z)) {
                    glPushMatrix();
                    glTranslatef(creature.location.x, creature.location.y, creature.location.z);
                    glCreature(creature);
                    glPopMatrix();
                }
            }
            glPopMatrix();

            // draw cursor
            if (df.cursor.x != invalid_location) {
                shaderprogram::no_program.apply();
                glPushAttrib(GL_ENABLE_BIT);
                glDisable(GL_LIGHTING);
                glDisable(GL_CULL_FACE);
                glPushMatrix();
                glTranslatef(df.cursor.x, df.cursor.y, df.cursor.z);
                TileDefinition def = df.map.tileDefinition(df.cursor.x, df.cursor.y, df.cursor.z);
                if (!def.hasfloor && def.bodytype == BodyType::none) { // empty space
                    glColor3f(.3, .3, .1);
                } else {
                    glColor3f(.7, .7, .2);
                }
                glCursor();
                glPopMatrix();
                glPopAttrib();
                shader_prog.apply();
            }

            // render liquid
            glPushMatrix();
            glTranslatef(0, 0, floor_height);
            for (auto z = range.begin(df.map.dimension.x - 1, df.map.dimension.y - 1, z_end, 1); z >= z_start; --z) {
                for (auto x = z.begin(1); x <= z.end(); ++x) {
                    for (auto y = x.begin(); y <= x.end(); ++y) {
                        if (!df.map.tileInfo(y.X(), y.Y(), y.Z()).invisible) {
                            glPushMatrix();
                            glTranslatef(y.X(), y.Y(), y.Z());
                            glLiquid(y.X(), y.Y(), y.Z());
                            glPopMatrix();
                        }
                    }
                }
            }
            glPopMatrix();
        }

        shaderprogram::no_program.apply();

        if (df.cursor.x != invalid_location) {
            glPushAttrib(GL_ENABLE_BIT);
            glPushMatrix();
            if (hud_repaint) {
                hud_repaint = false;
                hudDraw();
            }
            glEnable(GL_TEXTURE_2D);
            glDisable(GL_LIGHTING);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, HUD_WIDTH, HUD_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, painted_hud);
            glTranslatef(df.cursor.x,
                         df.cursor.y,
                         df.cursor.z);
            glRotatef(view_angle / M_PI * 180, 0, 0, 1);
            glColor3f(1, 1, 1);
            glBegin(GL_QUADS);
            glTexCoord2f(0, 1);
            glVertex3f(0, -HUD_LEFT , HUD_BOTTOM);
            glTexCoord2f(1, 1);
            glVertex3f(0, -HUD_RIGHT, HUD_BOTTOM);
            glTexCoord2f(1, 0);
            glVertex3f(0, -HUD_RIGHT, HUD_TOP);
            glTexCoord2f(0, 0);
            glVertex3f(0, -HUD_LEFT , HUD_TOP);
            glEnd();
            glPopMatrix();
            glPopAttrib();
        }

        glPopAttrib();
    }   break;
    default:
        break;
    }

    if (df.fps_on) {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixd(matrix_ortho);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glPushAttrib(GL_ENABLE_BIT);
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_TEXTURE_2D);
        stringstream s;
        {
            s << "FPS: " << df.ticks_per_sec << " (" << df.frames_per_sec << ")";
            string const & str = s.str();
            TEXT_SIZE size = glText(str.data(), str.length());
            float width = height * size.strRatio;
            float l = height, r = l + width;
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);
            glColor3fv(df.getConsoleColor(ConsoleColor::cyan, 0));
            glVertex2f(l, 0);
            glVertex2f(l, height);
            glVertex2f(r, height);
            glVertex2f(r, 0);
            glEnd();
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);
            glColor3fv(df.getConsoleColor(ConsoleColor::white, 0));
            glTexCoord2f(0, 0);                 glVertex2f(l, 0);
            glTexCoord2f(0, size.texH);         glVertex2f(l, height);
            glTexCoord2f(size.texW, size.texH); glVertex2f(r, height);
            glTexCoord2f(size.texW, 0);         glVertex2f(r, 0);
            glEnd();
        }
        s.str(""); s.clear();
        time = clock() - time;
        {
            s << (float) time / CLOCKS_PER_SEC * 1000 << "ms";
            string const & str = s.str();
            TEXT_SIZE size = glText(str.data(), str.length());
            float width = height * size.strRatio;
            float l = height, r = l + width;
            float u = height, d = height * 2;
            glDisable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);
            glColor3fv(df.getConsoleColor(ConsoleColor::cyan, 0));
            glVertex2f(l, u);
            glVertex2f(l, d);
            glVertex2f(r, d);
            glVertex2f(r, u);
            glEnd();
            glEnable(GL_TEXTURE_2D);
            glBegin(GL_QUADS);
            glColor3fv(df.getConsoleColor(ConsoleColor::white, 0));
            glTexCoord2f(0, 0);                 glVertex2f(l, u);
            glTexCoord2f(0, size.texH);         glVertex2f(l, d);
            glTexCoord2f(size.texW, size.texH); glVertex2f(r, d);
            glTexCoord2f(size.texW, 0);         glVertex2f(r, u);
            glEnd();
        }
        glPopAttrib();
    }
    return true;
}

void hudRepaint() {
    hud_repaint = true;
}

void hudDraw() {
    dword * bitmap = (dword *) painted_hud;
    dword datasize = HUD_WIDTH * HUD_HEIGHT;
    fill_n(bitmap, datasize, 0x20202020);
    SetTextColor(hud, 0x00FFFFFF);

    static ostringstream stream;
    static string text;
    int x = df.cursor.x, y = df.cursor.y, z = df.cursor.z;
    if (x != invalid_location) {
        if (df.side_menu_ui == UI::Fortress::look_around) {
            stream << "(" << x << ", " << y << ", " << z << ")";
            text = stream.str();
            TextOut(hud, 0, 0, text.data(), text.length());
            stream.clear();
            stream.str("");
            stream << "structure = " << hex << word(df.map.tileStructure(x, y, z)) << dec;
            text = stream.str();
            TextOut(hud, 0, 40, text.data(), text.length());
            stream.clear();
            stream.str("");

//            glPushMatrix();
//            glTranslatef(-center_x, -center_y, -center_z);
//            GLdouble projection[16], modelview[16];
//            GLint v[4] = {0, 0, 1, 1};
//            glGetDoublev(GL_PROJECTION_MATRIX, projection);
//            glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
//            glPopMatrix();
//            int ty = 0;
//            double zlow = center_z;
//            double zhigh = center_z + 10.7;
//            vector2d hv[2][2], lv[2][2];
//            double x0, y0, z0, x1, y1, z1, tmp;
//
//            stream << fixed << setprecision(1);
//
//            gluUnProject(0, 1, 1, modelview, projection, v, &x0, &y0, &z0);
//            gluUnProject(0, 1, 2, modelview, projection, v, &x1, &y1, &z1);
//            tmp = (zlow - z0) / (z1 - z0);
//            lv[0][0].x = x0 + (x1 - x0) * tmp;
//            lv[0][0].y = y0 + (y1 - y0) * tmp;
//            tmp = (zhigh - z0) / (z1 - z0);
//            hv[0][0].x = x0 + (x1 - x0) * tmp;
//            hv[0][0].y = y0 + (y1 - y0) * tmp;
//
//            gluUnProject(1, 1, 1, modelview, projection, v, &x0, &y0, &z0);
//            gluUnProject(1, 1, 2, modelview, projection, v, &x1, &y1, &z1);
//            tmp = (zlow - z0) / (z1 - z0);
//            lv[0][1].x = x0 + (x1 - x0) * tmp;
//            lv[0][1].y = y0 + (y1 - y0) * tmp;
//            tmp = (zhigh - z0) / (z1 - z0);
//            hv[0][1].x = x0 + (x1 - x0) * tmp;
//            hv[0][1].y = y0 + (y1 - y0) * tmp;
//
//            gluUnProject(0, 0, 1, modelview, projection, v, &x0, &y0, &z0);
//            gluUnProject(0, 0, 2, modelview, projection, v, &x1, &y1, &z1);
//            tmp = (zlow - z0) / (z1 - z0);
//            lv[1][0].x = x0 + (x1 - x0) * tmp;
//            lv[1][0].y = y0 + (y1 - y0) * tmp;
//            tmp = (zhigh - z0) / (z1 - z0);
//            hv[1][0].x = x0 + (x1 - x0) * tmp;
//            hv[1][0].y = y0 + (y1 - y0) * tmp;
//
//            gluUnProject(1, 0, 1, modelview, projection, v, &x0, &y0, &z0);
//            gluUnProject(1, 0, 2, modelview, projection, v, &x1, &y1, &z1);
//            tmp = (zlow - z0) / (z1 - z0);
//            lv[1][1].x = x0 + (x1 - x0) * tmp;
//            lv[1][1].y = y0 + (y1 - y0) * tmp;
//            tmp = (zhigh - z0) / (z1 - z0);
//            hv[1][1].x = x0 + (x1 - x0) * tmp;
//            hv[1][1].y = y0 + (y1 - y0) * tmp;
//
//            hv[1][0] = lineIntersectionPoint(hv[0][0], hv[1][0], lv[1][0], lv[1][1]);
//            hv[1][1] = lineIntersectionPoint(hv[0][1], hv[1][1], lv[1][0], lv[1][1]);
//            hv[0][0] += {.5, .5};
//            hv[0][1] += {-HUD_RIGHT, .5};
//            hv[1][0] += {.5, -.5};
//            hv[1][1] += {-HUD_RIGHT, -.5};
//
//            vector2d cursor = {df.cursor.x + .5, df.cursor.y + .5}, center = {center_x, center_y};
//            bool hleft   = (cursor - hv[1][0]).cross(hv[0][0] - hv[1][0]) < 0,
//                 hright  = (cursor - hv[1][1]).cross(hv[0][1] - hv[1][1]) > 0,
//                 vtop    = (cursor - hv[0][0]).cross(hv[0][1] - hv[0][0]) < 0,
//                 vbottom = (cursor - hv[1][0]).cross(hv[1][1] - hv[1][0]) > 0;
//            if (hleft || hright || vtop || vbottom) {
//                if (hleft && vtop) {
//                    center += (cursor - hv[0][0]);
//                } else if (hright && vtop) {
//                    center += (cursor - hv[0][1]);
//                } else if (hleft && vbottom) {
//                    center += (cursor - hv[1][0]);
//                } else if (hright && vbottom) {
//                    center += (cursor - hv[1][1]);
//                } else if (hleft || hright) {
//                    vector2d hint = lineIntersectionPoint(hv[0][0], hv[0][1], hv[1][0], hv[1][1]);
//
//                }
//            }
//
//            stream << "(" << x0 << ", " << y0 << ", " << zlow << ")";
//            text = stream.str(); TextOut(hud, 0, ty, text.data(), text.length()); stream.clear(); stream.str(""); ty += 40;
//
//            stream << "(" << x << ", " << y << ", " << z <<")";
//            text = stream.str(); TextOut(hud, 0, ty, text.data(), text.length()); stream.clear(); stream.str(""); ty += 40;
        }
    }
    while (datasize) {
        *bitmap ^= 0xFF000000;
        ++bitmap;
        --datasize;
    }
}

#endif

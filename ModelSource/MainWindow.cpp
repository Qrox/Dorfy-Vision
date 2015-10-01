#ifdef BUILD_MODEL

#include <iostream>
#include <fstream>
#include <algorithm>
#include <windowsx.h>
#include <gl/glu.h>
#include <cmath>
#include <typeinfo>
#include "Graphics.h"
#include "MainWindow.h"

#undef M_PI
double constexpr M_PI = 3.14159265358979323846;

using namespace std;

HWND modelWindow = nullptr;
ATOM registeredWindowClass = 0;
char const * windowClassName = "Model";
char const * windowTitle = "Model";

LRESULT CALLBACK MainWndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) noexcept ;

int windowInit() {
    HINSTANCE module = GetModuleHandle(NULL);
    WNDCLASS windowClass = {
        CS_HREDRAW | CS_VREDRAW | CS_OWNDC,
        MainWndProc,
        0,
        0,
        module,
        NULL,
        LoadCursor(NULL, IDC_ARROW),
        NULL,
        NULL,
        windowClassName,
    };
    registeredWindowClass = RegisterClass(&windowClass);
    modelWindow = CreateWindow(
        //(LPCTSTR) (DWORD) registeredWindowClass,
        windowClassName,
        windowTitle,
        WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (HWND) NULL,
        (HMENU) NULL,
        module,
        NULL
    );
    if (modelWindow == NULL) {
        return 0;
    } else {
        return 1;
    }
}

VOID CALLBACK RepaintTimer(HWND, UINT, UINT, DWORD) {
    if (glRepaint()) {
        SwapBuffers(hdc);
    }
}

int startMessageLoop() {
    MSG msg;
    SetTimer(modelWindow, 0, 1000 / 60, RepaintTimer);
    int go_on = 1;
    do {
        go_on = GetMessage(&msg, (HWND) NULL, 0, 0);
        if (go_on >= 0) { //no error
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    } while (go_on); //not WM_QUIT
    return 1;
}

BOOL lock_mouse = true;

void restoreMouseToLockedLocation(HWND window, SHORT & x, SHORT & y) {
    if (lock_mouse) {
        RECT clientRect;
        POINT pos;
        GetClientRect(window, &clientRect);
        x = pos.x = (clientRect.right - clientRect.left) >> 1;
        y = pos.y = (clientRect.bottom - clientRect.top) >> 1;
        ClientToScreen(window, &pos);
        SetCursorPos(pos.x, pos.y);
    }
}

bool unlockedMousePos = false;
SHORT unlockedMouseX, unlockedMouseY;

void lockMouse(HWND window, SHORT & x, SHORT & y) {
    if (!lock_mouse) {
        lock_mouse = true;
        while (ShowCursor(FALSE) >= 0);
        unlockedMouseX = x;
        unlockedMouseY = y;
        unlockedMousePos = true;
        restoreMouseToLockedLocation(window, x, y);
    }
}

void unlockMouse(HWND window) {
    if (lock_mouse) {
        lock_mouse = false;
        if (unlockedMousePos) {
            POINT pos = {unlockedMouseX, unlockedMouseY};
            ClientToScreen(window, &pos);
            SetCursorPos(pos.x, pos.y);
        }
        while (ShowCursor(TRUE) < 0);
    }
}

dword cmdcursor = 0;
string cmdline;

dword safecmdcursor() {
    if (cmdcursor > cmdline.length()) cmdcursor = cmdline.length();
    return cmdcursor;
}

// do not use vector3f::zero, etc, as they seem not init'd when init'ing this set
set<primitive *, primitivecomp> primitives;

vector<primitiveselection> selectedprimitives;
dword selectionindex = 0;

bool point_output = false, line_output = false;

void calcselection(HWND window, SHORT x, SHORT y) {
    RECT client;
    GetClientRect(window, &client);

    selectedprimitives.clear();
    for (auto i = primitives.begin(); i != primitives.end(); ++i) {
        primitive & prim = **i;
        float scrz;
        if (prim.isselected(x, client.bottom - client.top - y, matrix_modelview3d, matrix_frustum, gl_viewport, scrz)) {
            selectedprimitives.push_back(primitiveselection(scrz, &prim));
        }
    }
    sort(selectedprimitives.begin(), selectedprimitives.end(), [] (primitiveselection const & a, primitiveselection const & b) -> bool {
        return a.scrz < b.scrz;
    });
    selectionindex = 0;
}

void outputallatonce(ofstream & out, set<primitive *, primitivecomp> & primitives, primitive::primitivetype t) {
    // todo: insert points to vector, sort by normal x, y, z
    //       insert normals to set, sort by x, y, z
    //       cnt = point count + distinctive normal count (= normal set size)
    auto i = primitives.begin();
    for (; i != primitives.end(); ++i) {
        if ((**i).type() == t) {
            break;
        }
    }
    if (i != primitives.end()) {
        primitive::glbegin(out, (word) t);
        (**i).glrender(out);
        ++i;
        for (; i != primitives.end(); ++i) {
            primitive & prim = **i;
            if (prim.type() == t) {
                prim.glrender(out);
            }
        }
        primitive::glend(out);
    }
}

void executeCommand(string cmdline) {
    static vector<string> cmds;
    cmds.clear();
    auto pos = cmdline.find_first_not_of(' ');
    auto len = cmdline.find_last_not_of(' ') + 1;
    while (pos < len) {
        auto current = pos;
        while (true) {
            auto endorleft = cmdline.find_first_of(" <([{'\"", current); // end of the parameter, or start of a pair of brackets or quotes
            char endorleftchar, rightchar;
            if (endorleft == string::npos) { // end of cmdline, thus end of the parameter
                endorleft = len;
                endorleftchar = ' ';
            } else {
                endorleftchar = cmdline[endorleft];
            }
            if (endorleftchar == ' ') { // end of the parameter
                cmds.push_back(cmdline.substr(pos, endorleft - pos));
                pos = cmdline.find_first_not_of(' ', endorleft + 1);
                if (pos == string::npos) {
                    pos = len;
                }
                break;
            }
            switch (endorleftchar) { // start of a pair of brackets or quotes
            case '<': rightchar = '>'; break;
            case '(': rightchar = ')'; break;
            case '[': rightchar = ']'; break;
            case '{': rightchar = '}'; break;
            case '\'': rightchar = '\''; break;
            case '"': rightchar = '"'; break;
            }
            auto right = cmdline.find(rightchar, endorleft + 1);
            if (right == string::npos) { // incomplete brackets, treat as a parameter to the end
                current = len;
            } else { // a pair of brackets, continue from next char
                current = right + 1;
            }
        }
    }

    if (!cmds.empty()) {
        for (auto i = cmds.begin();;) {
            cout << *i;
            if (++i < cmds.end()) {
                cout << ' ';
            } else {
                break;
            }
        }
        cout << endl;

        string & cmd = cmds[0];
        if (cmd == "save") {
            if (cmd.size() > 1) {
                string file = cmds[1];
                if (file.length() < 4 || file.substr(file.length() - 4) != ".mdl") {
                    file += ".mdl";
                }
                ofstream out("models/" + file, ios_base::out | ios_base::trunc | ios_base::binary);
                float size[3] = {0, 0, 0};
                for (auto i = primitives.begin(); i != primitives.end(); ++i) {
                    primitive & prim = **i;
                    word type = prim.type();
                    if ((type != primitive::points || point_output) &&
                        (type != primitive:: lines ||  line_output)) {
                        vector3f sprim = prim.getsizefromorigin();
                        size[0] = max(size[0], sprim[0]);
                        size[1] = max(size[1], sprim[1]);
                        size[2] = max(size[2], sprim[2]);
                    }
                }
                out.write((char *) size, sizeof(size));

                if (point_output) outputallatonce(out, primitives, primitive::points);
                if (line_output) outputallatonce(out, primitives, primitive::lines);
                outputallatonce(out, primitives, primitive::triangles);
                outputallatonce(out, primitives, primitive::quads);
                for (auto i = primitives.begin(); i != primitives.end(); ++i) {
                    primitive & prim = **i;
                    primitive::primitivetype type = prim.type();
                    if (type != primitive::points && type != primitive::lines && type != primitive::triangles && type != primitive::quads) {
                        prim.glbegin(out);
                        prim.glrender(out);
                        prim.glend(out);
                    }
                }

                out.close();
            } else {
                cout << "usage: save <filename>" << endl;
            }
        } else if (cmd == "light") {
            if (cmds.size() > 2) {
                string type = cmds[1];
                vector3f pos = cmds[2];
                if (type == "point") {
                    lightpos[0] = pos[0];
                    lightpos[1] = pos[1];
                    lightpos[2] = pos[2];
                    lightpos[3] = 1;
                } else if (type == "far") {
                    lightpos[0] = pos[0];
                    lightpos[1] = pos[1];
                    lightpos[2] = pos[2];
                    lightpos[3] = 0;
                } else {
                    cout << "usage: light <point|far> <vertex>" << endl;
                }
            } else {
                cout << "usage: light <point|far> <vertex>" << endl;
            }
        } else if (cmd == "config") {
            if (cmds.size() > 1) {
                string & config = cmds[1];
                if (config == "output") {
                    if (cmds.size() > 3) {
                        string & type = cmds[2];
                        string & action = cmds[3];
                        bool fail;
                        bool enabled;
                        if (action == "enable") {
                            enabled = true;
                        } else if (action == "disable") {
                            enabled = false;
                        } else {
                            cout << "usage: config output " << type << " <enable|disable>" << endl;
                            fail = true;
                        }
                        if (!fail) {
                            if (type == "point") {
                                point_output = enabled;
                            } else if (type == "line") {
                                line_output = enabled;
                            } else {
                                cout << "usage: config output <point|line|triangle|quad> " << action << endl;
                            }
                        }
                    } else {
                        cout << "usage: config output <point|line|triangle|quad> <enable|disable>" << endl;
                    }
                } else {
                    cout << "usage: config <ouput> <parameters> ..." << endl;
                }
            } else {
                cout << "usage: config <ouput> <parameters> ..." << endl;
            }
        } else if (cmd == "remove_all") {
            if (cmds.size() > 1) {
                for (string::size_type i = 1; i < cmds.size(); ++i) {
                    string stype = cmds[i];
                    primitive::primitivetype type;
                    if (stype == "point" || stype == "points") {
                        type = primitive::points;
                    } else if (stype == "line" || stype == "lines") {
                        type = primitive::lines;
                    } else if (stype == "line_loop" || stype == "line_loops") {
                        type = primitive::line_loop;
                    } else if (stype == "line_strip" || stype == "line_strips") {
                        type = primitive::line_strip;
                    } else if (stype == "triangle" || stype == "triangles") {
                        type = primitive::triangles;
                    } else if (stype == "triangle_strip" || stype == "triangle_strips") {
                        type = primitive::triangle_strip;
                    } else if (stype == "triangle_fan" || stype == "triangle_fans") {
                        type = primitive::triangle_fan;
                    } else if (stype == "quad" || stype == "quads") {
                        type = primitive::quads;
                    } else if (stype == "quad_strip" || stype == "quad_strips") {
                        type = primitive::quad_strip;
                    } else if (stype == "polygon" || stype == "polygons") {
                        type = primitive::polygon;
                    } else {
                        cout << "unknown type " << stype << endl;
                        continue;
                    }
                    for (auto i = primitives.begin(); i != primitives.end();) {
                        if ((**i).type() == type) {
                            delete *i;
                            i = primitives.erase(i);
                        } else {
                            ++i;
                        }
                    }
                }
            } else {
                cout << "usage: remove_all <points|lines|...> ..." << endl;
            }
        } else if (cmd == "clear") {
            for (auto i = primitives.begin(); i != primitives.end(); ++i) {
                delete *i;
            }
            primitives.clear();
        } else if (cmd == "point" || cmd == "points") {
            if (cmds.size() > 1) {
                for (string::size_type i = 1; i < cmds.size(); ++i) {
                    primitives.insert(new point(cmds[i]));
                }
            } else {
                cout << "usage: point <vertex> ..." << endl;
            }
        } else if (cmd == "split") {
            if (cmds.size() > 2) {
                vector3f a(cmds[1]), b(cmds[2]);
                long cnt;
                if (cmds.size() > 3) {
                    cnt = strtol(cmds[3].data(), nullptr, 10);
                } else {
                    cnt = 2;
                }
                if (cnt >= 2 && cnt <= 10) {
                    vector3f d = (b - a) / cnt;
                    for (dword i = 1; i < (dword) cnt; ++i) {
                        // todo normal
                        primitives.insert(new point(a + d * i));
                    }
                } else {
                    cout << "usage: split <vertex> <vertex> [count 2~10]" << endl;
                }
            } else {
                cout << "usage: split <vertex> <vertex> [count 2~10]" << endl;
            }
        } else if (cmd == "cut") {
            if (cmds.size() > 3) {
                vector3f a(cmds[1]), b(cmds[2]);
                float porpotion = strtof(cmds[3].data(), nullptr);
                primitives.insert(new point(a + (b - a) * porpotion));
            } else {
                cout << "usage: cut <vertex> <vertex> <porpotion>" << endl;
            }
        } else if (cmd == "arc_split") {
            if (cmds.size() > 3) {
                vector3f o(cmds[1]), a(cmds[2]), b(cmds[3]);
                long cnt;
                if (cmds.size() > 4) {
                    cnt = strtol(cmds[4].data(), nullptr, 10);
                } else {
                    cnt = 2;
                }
                if (cnt >= 2 && cnt <= 10) {
                    vector3f va = a - o, vb = b - o;
                    vector3f dang = va.angleto(vb) / cnt;
                    float la = va.mod(), dl = (vb.mod() - la) / cnt;
                    vector3f e0 = va / la;
                    for (dword i = 1; i < (dword) cnt; ++i) {
                        primitives.insert(new point(o + e0.rotate(dang * i) * (la + dl * i)));
                    }
                } else {
                    cout << "usage: arc_split <center> <side a> <side b> [count 2 ~ 10]" << endl;
                }
            } else {
                cout << "usage: arc_split <center> <side a> <side b> [count 2 ~ 10]" << endl;
            }
        } else if (cmd == "arc_cut") {
            if (cmds.size() > 4) {
                vector3f o(cmds[1]), a(cmds[2]), b(cmds[3]);
                float porpotion = strtof(cmds[4].data(), nullptr);
                vector3f va = a - o, vb = b - o;
                vector3f dang = va.angleto(vb) * porpotion;
                float la = va.mod(), l = la + (vb.mod() - la) * porpotion;
                vector3f e0 = va / la;
                primitives.insert(new point(o + e0.rotate(dang) * l));
            } else {
                cout << "usage: arc_cut <center> <side a> <side b> <porpotion>" << endl;
            }
        } else if (cmd == "line" || cmd == "lines") {
            if (cmds.size() > 2) {
                if ((cmds.size() - 1) & 1) {
                    cout << "the last unpaired vertex ignored" << endl;
                }
                for (string::size_type i = 1; i + 1 < cmds.size(); i += 2) {
                    primitives.insert(new line(cmds[i], cmds[i + 1]));
                }
            } else {
                cout << "usage: line <vertex>*2*n" << endl;
            }
        } else if (cmd == "normal") {
            if (cmds.size() > 3) {
                vector3f origin(cmds[1]);
                vector3f normal;
                vector3f surf0(cmds[2]);
                for (string::size_type i = 3; i < cmds.size(); ++i) {
                    vector3f surf1(cmds[i]);
                    normal = normal + (surf1 - origin).cross(surf0 - origin);
                    surf0 = surf1;
                }
                if (cmds.size() > 4) {
                    vector3f surf1(cmds[2]);
                    normal = normal + (surf1 - origin).cross(surf0 - origin);
                }
                normal = normal.normalize() / 10;
                if (normal.dot(normal) != 0) {
                    primitives.insert(new line(origin, origin + normal));
                } else {
                    cout << "normal is of zero length!" << endl;
                }
            } else if (cmds.size() > 2){
                vector3f origin(cmds[1]);
                vector3f end(cmds[2]);
                vector3f normal = (end - origin).normalize() / 10;
                if (normal.dot(normal) != 0) {
                    primitives.insert(new line(origin, origin + normal));
                } else {
                    cout << "normal is of zero length!" << endl;
                }
            } else {
                cout << "usage: normal <vertex> <surface vertex>*2+" << endl;
                cout << "or     normal <vertex> <normal end>" << endl;
            }
        } else if (cmd == "triangle" || cmd == "triangles") {
            if (cmds.size() > 3) {
                auto ignored = (cmds.size() - 1) % 3;
                if (ignored) {
                    cout << (ignored == 1 ? "the last ungrouped vertex ignored" : "the last two ungrouped vertices ignored") << endl;
                }
                for (string::size_type i = 1; i + 2 < cmds.size(); i += 3) {
                    primitives.insert(new triangle(cmds[i], cmds[i + 1], cmds[i + 2]));
                }
            } else {
                cout << "usage: triangle <vertex>*3*n" << endl;
            }
        } else if (cmd == "quad" || cmd == "quads") {
            if (cmds.size() > 4) {
                auto ignored = (cmds.size() - 1) & 3;
                if (ignored == 1) {
                    cout << "the last ungrouped vertex ignored" << endl;
                } else if (ignored > 1) {
                    cout << "the last " << ignored << " vertices ignored" << endl;
                }
                for (string::size_type i = 1; i + 3 < cmds.size(); i += 4) {
                    primitives.insert(new quad(cmds[i], cmds[i + 1], cmds[i + 2], cmds[i + 3]));
                }
            } else {
                cout << "usage: quad <vertex>*4*n" << endl;
            }
        } else if (cmd == "line_loop") {
            if (cmds.size() > 2) {
                primitives.insert(new line_loop(cmds.begin() + 1, cmds.end()));
            } else {
                cout << "usage: line_loop <vertex>*2+" << endl;
            }
        } else if (cmd == "line_strip") {
            if (cmds.size() > 2) {
                primitives.insert(new line_strip(cmds.begin() + 1, cmds.end()));
            } else {
                cout << "usage: line_strip <vertex>*2+" << endl;
            }
        } else if (cmd == "triangle_strip") {
            if (cmds.size() > 3) {
                primitives.insert(new triangle_strip(cmds.begin() + 1, cmds.end()));
            } else {
                cout << "usage: triangle_strip <vertex>*3+" << endl;
            }
        } else if (cmd == "triangle_fan") {
            if (cmds.size() > 3) {
                primitives.insert(new triangle_fan(cmds.begin() + 1, cmds.end()));
            } else {
                cout << "usage: triangle_fan <vertex>*3+" << endl;
            }
        } else if (cmd == "quad_strip") {
            if (cmds.size() > 4) {
                if ((cmds.size() - 1) & 1) {
                    cout << "the last unpaired vertex ignored" << endl;
                }
                primitives.insert(new quad_strip(cmds.begin() + 1, cmds.end()));
            } else {
                cout << "usage: quad_strip <vertex>*2*2+" << endl;
            }
        } else if (cmd == "polygon") {
            if (cmds.size() > 3) {
                primitives.insert(new polygon(cmds.begin() + 1, cmds.end()));
            } else {
                cout << "usage: polygon <vertex>*3+";
            }
        } else if (cmd == "ellipsoid") {
            if (cmds.size() > 5) {
                vector3f o(cmds[1]), x(cmds[2]), y(cmds[3]), z(cmds[4]);
                x = x - o;
                y = y - o;
                z = z - o;
                long prec = strtol(cmds[5].data(), nullptr, 10);
                if (prec <= 2 || prec > 10) {
                    cout << "usage: ellipsoid <center> <x end> <y end> <z end> [precision 3 ~ 10]" << endl;
                }
                dword thetacnt = prec, phicnt = thetacnt / 2;
                dword phii = 0;
                float cosphi0, sinphi0, cosphi1, sinphi1;
                vector<vector3f> vs;
                vector<vector3f> ns;

                cosphi1 = cos(M_PI / phicnt * (phii + 1));
                sinphi1 = sin(M_PI / phicnt * (phii + 1));
                vs.push_back(o - z);
                ns.push_back(y.cross(x).normalize());
                for (dword thetai = 0; thetai <= thetacnt; ++thetai) {
                    float costheta = cos(M_PI * 2 / thetacnt * thetai);
                    float sintheta = sin(M_PI * 2 / thetacnt * thetai);
                    vs.push_back(o + x * sinphi1 * costheta + y * sinphi1 * sintheta - z * cosphi1);
                    ns.push_back((y * costheta - x * sintheta)
                        .cross(x * cosphi1 * costheta + y * cosphi1 * sintheta + z * sinphi1).normalize());
                }
                primitives.insert(new triangle_fan(vs, ns));
                vs.clear();
                ns.clear();

                for (phii = 1; phii < phicnt - 1; ++phii) {
                    cosphi0 = cos(M_PI / phicnt * phii);
                    sinphi0 = sin(M_PI / phicnt * phii);
                    cosphi1 = cos(M_PI / phicnt * (phii + 1));
                    sinphi1 = sin(M_PI / phicnt * (phii + 1));
                    for (dword thetai = 0; thetai <= thetacnt; ++thetai) {
                        float costheta = cos(M_PI * 2 / thetacnt * thetai);
                        float sintheta = sin(M_PI * 2 / thetacnt * thetai);
                        vs.push_back(o + x * sinphi0 * costheta + y * sinphi0 * sintheta - z * cosphi0);
                        ns.push_back((y * costheta - x * sintheta)
                            .cross(x * cosphi0 * costheta + y * cosphi0 * sintheta + z * sinphi0).normalize());
                        vs.push_back(o + x * sinphi1 * costheta + y * sinphi1 * sintheta - z * cosphi1);
                        ns.push_back((y * costheta - x * sintheta)
                            .cross(x * cosphi1 * costheta + y * cosphi1 * sintheta + z * sinphi1).normalize());
                    }
                    primitives.insert(new triangle_strip(vs, ns));
                    vs.clear();
                    ns.clear();
                }

                cosphi0 = cos(M_PI / phicnt * phii);
                sinphi0 = sin(M_PI / phicnt * phii);
                vs.push_back(o + z);
                ns.push_back(x.cross(y).normalize());
                for (int32 thetai = thetacnt; thetai >= 0; --thetai) {
                    float costheta = cos(M_PI * 2 / thetacnt * thetai);
                    float sintheta = sin(M_PI * 2 / thetacnt * thetai);
                    vs.push_back(o + x * sinphi0 * costheta + y * sinphi0 * sintheta - z * cosphi0);
                    ns.push_back((y * costheta - x * sintheta)
                        .cross(x * cosphi0 * costheta + y * cosphi0 * sintheta + z * sinphi0).normalize());
                }
                primitives.insert(new triangle_fan(vs, ns));
            } else {
                cout << "usage: ellipsoid <center> <x semiaxis> <y semiaxis> <z semiaxis> [precision 3 ~ 10]" << endl;
            }
        } else if (cmd == "briolette") {
            if (cmds.size() > 1) {
                dword constexpr phicnt = 6, thetacnt = 12;
                float height = strtof(cmds[1].data(), nullptr);
                float scale = height / (pow(M_PI, 4) - 5 * pow(M_PI, 2) + 100);
                float rbottom = 50 * scale;
                for (dword phii = 0; phii < phicnt; ++phii) {
                    float phi0 = M_PI / phicnt * phii;
                    float phi1 = M_PI / phicnt * (phii + 1);
                    float cosphi0 = cos(phi0);
                    float sinphi0 = sin(phi0);
                    float cosphi1 = cos(phi1);
                    float sinphi1 = sin(phi1);
                    float r0 = (phi0 * phi0 * phi0 * phi0 - 5 * phi0 * phi0 + 50) * scale;
                    float r1 = (phi1 * phi1 * phi1 * phi1 - 5 * phi1 * phi1 + 50) * scale;
                    for (dword thetai = 0; thetai < thetacnt; ++thetai) {
                        float costheta00 = cos(M_PI * 2 / thetacnt * (thetai + (phii & 1 ? 0 :  .5)));
                        float sintheta00 = sin(M_PI * 2 / thetacnt * (thetai + (phii & 1 ? 0 :  .5)));
                        float costheta01 = cos(M_PI * 2 / thetacnt * (thetai + (phii & 1 ? 1 : 1.5)));
                        float sintheta01 = sin(M_PI * 2 / thetacnt * (thetai + (phii & 1 ? 1 : 1.5)));
                        vector3f v00(r0 * sinphi0 * costheta00, r0 * sinphi0 * sintheta00, -r0 * cosphi0 + rbottom);
                        vector3f v01(r0 * sinphi0 * costheta01, r0 * sinphi0 * sintheta01, -r0 * cosphi0 + rbottom);
                        float costheta10 = cos(M_PI * 2 / thetacnt * (thetai + (phii & 1 ?  .5 : 0)));
                        float sintheta10 = sin(M_PI * 2 / thetacnt * (thetai + (phii & 1 ?  .5 : 0)));
                        float costheta11 = cos(M_PI * 2 / thetacnt * (thetai + (phii & 1 ? 1.5 : 1)));
                        float sintheta11 = sin(M_PI * 2 / thetacnt * (thetai + (phii & 1 ? 1.5 : 1)));
                        vector3f v10(r1 * sinphi1 * costheta10, r1 * sinphi1 * sintheta10, -r1 * cosphi1 + rbottom);
                        vector3f v11(r1 * sinphi1 * costheta11, r1 * sinphi1 * sintheta11, -r1 * cosphi1 + rbottom);
                        vector3f n;
                        if (phii < phicnt - 1) {
                            vector3f & v0 = phii & 1 ? v01 : v00;
                            n = (v11 - v0).cross(v10 - v0).normalize();
                            primitives.insert(new triangle(v0, v10, v11, n, n, n));
                        }
                        if (phii > 0) {
                            vector3f & v1 = phii & 1 ? v10 : v11;
                            n = (v00 - v1).cross(v01 - v1).normalize();
                            primitives.insert(new triangle(v1, v01, v00, n, n, n));
                        }
                    }
                }
            }
        } else {
            cout << "unknown command " << cmd << endl;
        }
    }
}

LRESULT CALLBACK MainWndProc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam) noexcept {
    try {
        static BOOL first = true;
        static SHORT x0, y0;
        switch (msg) {
        case WM_CREATE:
            if (lock_mouse) {
                while (ShowCursor(FALSE) >= 0);
            }
            break;
        case WM_CLOSE:
            DestroyWindow(window);
            break;
        case WM_DESTROY:
            PostQuitMessage(0);
            break;
        case WM_QUIT:
            break;
        case WM_MOVE:
        case WM_SIZE: {
            RECT rect;
            if (GetClientRect(window, &rect)) {
                glResize(rect.right - rect.left, rect.bottom - rect. top);
            }
            if (lock_mouse) {
                restoreMouseToLockedLocation(window, x0, y0);
                first = false;
            }
        }   break;
        case WM_LBUTTONDOWN: {
            if (!lock_mouse) {
                bool lock = true;
                if (selectionindex < selectedprimitives.size()) {
                    switch (selectedprimitives[selectionindex].prim->type()) {
                        case primitive::points: {
                            point & prim = *(point *) selectedprimitives[selectionindex].prim;
                            string ins;
                            if (cmdline[cmdcursor - 1] != ' ') {
                                ins = " ";
                            }
                            ins += (string) prim.v;
                            if ((prim.n).dot(prim.n) != 0) ins += ":" + (string) prim.n;
                            ins += ' ';
                            cmdline.insert(safecmdcursor(), ins);
                            cmdcursor += ins.length();
                            lock = false;
                        }   break;
                        case primitive::lines: {
                            line & prim = *(line *) selectedprimitives[selectionindex].prim;
                            if (cmdcursor > 0 && cmdline[cmdcursor - 1] == ' ') {
                                safecmdcursor();
                                cmdline.erase(cmdcursor - 1, 1);
                                --cmdcursor;
                            }
                            string ins = ":" + (string) (prim.v[1] - prim.v[0]).normalize();
                            cmdline.insert(safecmdcursor(), ins);
                            cmdcursor += ins.length();
                            lock = false;
                        }   break;
                        default:
                            break;
                    }
                }
                if (lock) {
                    lockMouse(window, x0, y0);
                    selectedprimitives.clear();
                    first = false;
                }
            }
        }   break;
        case WM_RBUTTONDOWN: {
            if (!lock_mouse && selectionindex < selectedprimitives.size()) {
                delete selectedprimitives[selectionindex].prim; // delete primitive
                primitives.erase(selectedprimitives[selectionindex].prim); // remove pointer
                selectedprimitives.erase(selectedprimitives.begin() + selectionindex); // remove selection
                if (selectionindex > 0) {
                    --selectionindex;
                }
            }
        }   break;
        case WM_MOUSEMOVE: {
            SHORT x1, y1;
            x1 = GET_X_LPARAM(lparam);
            y1 = GET_Y_LPARAM(lparam);
            if (lock_mouse && !first && (x1 != x0 || y1 != y0)) {
                vector3f horizon = stance.cross(eye);
                vector3f movement = (horizon * (x0 - x1) + stance * (y0 - y1)) / 256;
                vector3f rotation = eye.cross(movement);
                eye = eye.rotate(rotation); //.normalize() if cannot keep unit length
                stance = stance.rotate(rotation); //.normalize() if cannot keep unit length
                //stance = eye.cross(stance.cross(eye)); // if cannot keep perpendicular
            } else if (!lock_mouse) {
                calcselection(window, x1, y1);
            }
            if (first || x1 != x0 || y1 != y0) {
                if (lock_mouse) {
                    restoreMouseToLockedLocation(window, x0, y0);
                    first = false;
                } else {
                    x0 = x1;
                    y0 = y1;
                    first = false;
                }
            }
        }   break;
        case WM_MOUSEWHEEL:
            if (!lock_mouse && !selectedprimitives.empty()) {
                auto delta = GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA;
                auto max = selectedprimitives.size();
                if (delta < 0) {
                    selectionindex += max - ((-delta) % max); // since selectionindex is unsigned, we need this to ensure continuous scrolling
                } else {
                    selectionindex += delta;
                }
                selectionindex %= max;
            } else if (lock_mouse) {
                float d = (float) GET_WHEEL_DELTA_WPARAM(wparam) / WHEEL_DELTA / 10;
                float sc = scale + d;
                if (sc < .1) {
                    sc = .1;
                } else if (sc > 10) {
                    sc = 10;
                }
                scale = sc;
            }
            break;
        case WM_KEYDOWN:
            switch (wparam) {
            case VK_ESCAPE:
                if (lock_mouse) {
                    unlockMouse(window);
                    calcselection(window, x0, y0);
                }
                break;
            case VK_RETURN:
                if (!cmdline.empty()) {
                    executeCommand(cmdline);
                    cmdline = "";
                    cmdcursor = 0;
                }
                break;
            case VK_BACK:
                if (cmdcursor > 0) {
                    safecmdcursor();
                    cmdline.erase(cmdcursor - 1, 1);
                    --cmdcursor;
                }
                break;
            case VK_LEFT:
                if (cmdcursor > 0) {
                    --cmdcursor;
                }
                break;
            case VK_RIGHT:
                if (cmdcursor < cmdline.length()) {
                    ++cmdcursor;
                }
                break;
            case VK_TAB: // switch between design rendering and actual rendering
                frustum = !frustum;
                if (frustum) {
                    cout << "switched to actual rendering" << endl;
                    glEnable(GL_CULL_FACE);
                } else {
                    cout << "switched to design rendering" << endl;
                    glDisable(GL_CULL_FACE);
                }
                RECT client;
                GetClientRect(window, &client);
                glResize(client.right - client.left, client.bottom - client.top);
                if (!lock_mouse) {
                    calcselection(window, x0, y0);
                }
                break;
            }   break;
        case WM_CHAR:
            char ch;
            if (isspace(wparam) && wparam != '\t' && wparam != '\n' && wparam != '\r') {
                ch = ' ';
            } else if (isgraph(wparam)) {
                ch = (char) wparam;
            } else {
                break;
            }
            cmdline.insert(safecmdcursor(), 1, ch);
            ++cmdcursor;
            break;
        default:
            return DefWindowProc(window, msg, wparam, lparam);
        }
    } catch (exception & ex) {
        cout << "exception in event loop: " << typeid(ex).name() << "@" << ex.what() << endl;
    } catch (...) {
        cout << "exception in event loop" << endl;
    }
    return 0;
}

#endif

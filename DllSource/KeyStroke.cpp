#ifdef BUILD_DLL

#include "gl/glew.h"
#include <windows.h>
#include <cmath>
#include "DwarfFortress.h"
#include "Graphics.h"
#include "Utils.h"
#include "Timers.h"
#include "KeyStroke.h"

using namespace DwarfFortress;
using namespace DwarfFortress::UI;

#define PREVIOUSLY_DOWN     (1 << 30)

#define KEY_SHIFT           0x00010000
#define KEY_CTRL            0x00020000
#define KEY_ALT             0x00040000
#define KEY_META            0x00080000
#define KEY_UP              0x00100000
#define KEY_DOWN            0x00200000
#define KEY_LEFT            0x00400000
#define KEY_RIGHT           0x00800000

DWORD modifier_current = 0;
DWORD key_current = 0;

bool key_executed = false;

void SendKeyToDwarf(DWORD modifier, DWORD key) {
    if (dfWindow) {
        DWORD lparam = 1; // repeat count for all key events // todo: scan code
        if (modifier & KEY_SHIFT) SendMessage(dfWindow, WM_KEYDOWN, VK_SHIFT, lparam);
        if (modifier & KEY_CTRL)  SendMessage(dfWindow, WM_KEYDOWN, VK_CONTROL, lparam);
        if (modifier & KEY_ALT)   SendMessage(dfWindow, WM_KEYDOWN, VK_MENU, lparam);
        if (modifier & KEY_META)  SendMessage(dfWindow, WM_KEYDOWN, VK_LWIN, lparam);
        if (modifier & KEY_UP)    SendMessage(dfWindow, WM_KEYDOWN, VK_UP, lparam);
        if (modifier & KEY_DOWN)  SendMessage(dfWindow, WM_KEYDOWN, VK_DOWN, lparam);
        if (modifier & KEY_LEFT)  SendMessage(dfWindow, WM_KEYDOWN, VK_LEFT, lparam);
        if (modifier & KEY_RIGHT) SendMessage(dfWindow, WM_KEYDOWN, VK_RIGHT, lparam);
        if (key)                  SendMessage(dfWindow, WM_KEYDOWN, key, lparam);
        lparam |= 0xC0000000; // previous down flag and transition state flag for KEYUP events
        if (key)                  SendMessage(dfWindow, WM_KEYUP, key, lparam);
        if (modifier & KEY_RIGHT) SendMessage(dfWindow, WM_KEYUP, VK_RIGHT, lparam);
        if (modifier & KEY_LEFT)  SendMessage(dfWindow, WM_KEYUP, VK_LEFT, lparam);
        if (modifier & KEY_DOWN)  SendMessage(dfWindow, WM_KEYUP, VK_DOWN, lparam);
        if (modifier & KEY_UP)    SendMessage(dfWindow, WM_KEYUP, VK_UP, lparam);
        if (modifier & KEY_META)  SendMessage(dfWindow, WM_KEYUP, VK_LWIN, lparam);
        if (modifier & KEY_ALT)   SendMessage(dfWindow, WM_KEYUP, VK_MENU, lparam);
        if (modifier & KEY_CTRL)  SendMessage(dfWindow, WM_KEYUP, VK_CONTROL, lparam);
        if (modifier & KEY_SHIFT) SendMessage(dfWindow, WM_KEYUP, VK_SHIFT, lparam);
    }
}

bool executeDirectionKey(DWORD modifier, DWORD key) {
    switch (df.getTopmostUI().getUItype()) {
    case dwarf_fortress: {
        float dir = view_angle;
        int dz = 0;
        float dist = modifier & KEY_SHIFT ? limit(1.f, view_radius / 8.f, 10.f) : 1.f; // todo

        bool is_dir_key = true, is_z_key = true;
        switch (modifier & ~KEY_SHIFT) {
        case KEY_DOWN:
        case KEY_DOWN | KEY_LEFT | KEY_RIGHT:
          //dir += M_PI * 0   ;
            break;
        case KEY_DOWN | KEY_LEFT:
            dir += M_PI * 0.25;
            break;
        case KEY_LEFT:
        case KEY_LEFT | KEY_UP | KEY_DOWN:
            dir += M_PI * 0.5 ;
            break;
        case KEY_UP | KEY_LEFT:
            dir += M_PI * 0.75;
            break;
        case KEY_UP:
        case KEY_UP | KEY_LEFT | KEY_RIGHT:
            dir += M_PI       ;
            break;
        case KEY_UP | KEY_RIGHT:
            dir += M_PI * 1.25;
            break;
        case KEY_RIGHT:
        case KEY_RIGHT | KEY_UP | KEY_DOWN:
            dir += M_PI * 1.5 ;
            break;
        case KEY_DOWN | KEY_RIGHT:
            dir += M_PI * 1.75;
            break;
        default:
            dist = 0;
            is_dir_key = false;
            break;
        }
        switch (key) {
        case VK_OEM_COMMA:
            dz =  1;
            break;
        case VK_OEM_PERIOD:
            dz = -1;
            break;
        default:
            is_z_key = false;
        }

        if (!is_dir_key && !is_z_key) {
            return false;
        }

        if (df.side_menu_ui == UI::Fortress::look_around) {
            df.cursor.x = limit(0, df.cursor.x + (int32) round(cos(dir) * dist), (int32) df.map.dimension.x - 1);
            df.cursor.y = limit(0, df.cursor.y + (int32) round(sin(dir) * dist), (int32) df.map.dimension.y - 1);
            df.cursor.z = limit(0, df.cursor.z + dz, (int32) df.map.dimension.z - 1);
            float a = sin(view_angle), b = cos(view_angle);
            float dx = df.cursor.x + .5 - center_x, dy = df.cursor.y + .5 - center_y;
            float chdist = b * dx + a * dy;
            float cvdist = a * dx - b * dy;
            float ohdist = limit(chdist - 10, 0.f, chdist - 1);
            float tmp = 5.5 / 9. * (chdist - ohdist - 1);
            float ovdist = limit(cvdist - 6 + tmp, 0.f, cvdist + 13 - tmp);
            center_x += ohdist * b + ovdist * a;
            center_y += ohdist * a - ovdist * b;
            center_z = df.cursor.z;
            hudRepaint();
        } else { // todo
            center_x = limit(-view_radius * .5f, center_x + cos(dir) * dist, df.map.dimension.x + view_radius * .5f);
            center_y = limit(-view_radius * .5f, center_y + sin(dir) * dist, df.map.dimension.y + view_radius * .5f);
            center_z = limit(0.f, center_z + dz, (float) df.map.dimension.z - 1);
        }
    }   return true;
    default:
        break;
    }
    return false;
}

#define ANGLE_ROTATE (M_PI / 16)

bool keyStroke(HWND window, DWORD modifier, DWORD key) {
    if ((modifier & (KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT)) || key == VK_OEM_COMMA || key == VK_OEM_PERIOD) { // contains direction keys
        if (executeDirectionKey(modifier, key)) {
            return true;
        }
    }
    bool executed = true;
    switch (modifier | key) {
    case VK_OEM_1:
        view_angle += ANGLE_ROTATE;
        view_angle -= floor(view_angle / M_PI / 2) * M_PI * 2;
        if (df.side_menu_ui == UI::Fortress::look_around && df.cursor.x != invalid_location) {
            vector2d cursor = {(double) df.cursor.x, (double) df.cursor.y}, center = {center_x, center_y};
            center = cursor + (center - cursor).rotate(ANGLE_ROTATE);
            center_x = center.x; center_y = center.y;
            //hudRepaint();
        }
        break;
    case VK_OEM_7:
        view_angle -= ANGLE_ROTATE;
        view_angle -= floor(view_angle / M_PI / 2) * M_PI * 2;
        if (df.side_menu_ui == UI::Fortress::look_around && df.cursor.x != invalid_location) {
            vector2d cursor = {(double) df.cursor.x, (double) df.cursor.y}, center = {center_x, center_y};
            center = cursor + (center - cursor).rotate(-ANGLE_ROTATE);
            center_x = center.x; center_y = center.y;
            //hudRepaint();
        }
        break;
    case VK_OEM_4:
        pitch_angle -= ANGLE_ROTATE;
        if (pitch_angle < .001) pitch_angle = .001;
        break;
    case VK_OEM_6:
        pitch_angle += ANGLE_ROTATE;
        if (pitch_angle > M_PI / 2 - .001) pitch_angle = M_PI / 2 - .001;
        break;
    case 'K':
        if (df.side_menu_ui == UI::Fortress::none) {
            df.cursor.x = center_x;
            df.cursor.y = center_y;
            df.cursor.z = center_z;
            df.side_menu_ui = UI::Fortress::look_around;
        }
        break;
    default:
        executed = false;
        break;
    }
    if (executed) {
        return true;
    }

#ifdef DEBUG
    //debug probe
    if (df.cursor.x != invalid_location) {
        int x = df.cursor.x, y = df.cursor.y, z = df.cursor.z;
        int lx = x & 15, ly = y & 15;
        Block3d & block = df.map.getBlock3d(x, y, z);
        if (&block) {
            executed = true;
            switch (modifier | key) { // the keystroke
            case 'P': // probe
                cout << "Probe on tile (" << x << ", " << y << ", " << z << ") :" << endl
                     << "Structure type is 0x" << hex << word(block.tile_structure[lx][ly]) << endl
                     << "Temperature 1 is " << block.temperature_1[lx][ly] << endl
                     << "Temperature 2 is " << block.temperature_2[lx][ly] << endl;
                break;
            case 'W': // place 1/7 water
                block.tile_info[lx][ly].liquid_level++;
                break;
            case KEY_SHIFT | 'W': // remove 1/7 water
                block.tile_info[lx][ly].liquid_level--;
                break;
            case 'S': // set block
                for (int x = 0; x < 16; x++) {
                    for (int y = 0; y < 16; y++) {
                        block.tile_structure[x][y] = TileStructure::obsidian_wall;
                    }
                }
                break;
            case 'X':
                for (int k = 0; k < 3; ++k) {
                    Block3d & block = df.map.getBlock3d(x, y + k * 16, z);
                    if (&block) {
                        for (int x = 0; x < 16; ++x) {
                            for (int y = 0; y < 16; ++y) {
                                block.tile_structure[x][y] = (TileStructure::Enum) ((k << 8) | (y << 4) | x);
                            }
                        }
                    }
                }
                break;
            default:
                executed = false;
                break;
            }
            if (executed) {
                return true;
            }
        }
    }
    switch (modifier | key) {
    case 'F': { // find first block
        bool proceed = true;
        word structure = 0x14B;
//        cout << "Input type: >> " << endl;
//        cin >> structure;
        for (dword x = 0; proceed && x < df.map.dimension.x; x += 16) {
            for (dword y = 0; proceed && y < df.map.dimension.y; y += 16) {
                for (dword z = 0; proceed && z < df.map.dimension.z; ++z) {
                    Block3d & block3d = df.map.getBlock3d(x, y, z);
                    if (&block3d) {
                        for (dword lx = 0; proceed && lx < 16; ++lx) {
                            for (dword ly = 0; proceed && ly < 16; ++ly) {
                                if (block3d.tile_structure[lx][ly] == structure) {
                                    df.view_z = z;
                                    df.view_x = x + lx - df.getFortressModeViewWidth() / 2;
                                    df.view_y = y + ly - df.getFortressModeViewHeight() / 2;
                                    proceed = false;
                                }
                            }
                        }
                    }
                }
            }
        }
    }   return true;
    case 'R': // reveal block
        for (dword x = 0; x < df.map.dimension.x; x += 16) {
            for (dword y = 0; y < df.map.dimension.y; y += 16) {
                for (dword z = 0; z < df.map.dimension.z; ++z) {
                    Block3d & block3d = df.map.getBlock3d(x, y, z);
                    if (&block3d) {
                        for (dword lx = 0; lx < 16; ++lx) {
                            for (dword ly = 0; ly < 16; ++ly) {
                                block3d.tile_info[lx][ly].invisible = false;
                            }
                        }
                    }
                }
            }
        }
        return true;
    case 'Q':
        shader_prog.detach(shader_vert);
        shader_prog.detach(shader_frag);
        shader_vert.destroy();
        shader_frag.destroy();
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
        return true;
    }
#endif
    return false;
}

void keyUp(HWND window, WPARAM wparam, LPARAM lparam) {
    if (!key_executed) {
        if (!keyStroke(window, modifier_current, key_current)) {
            SendKeyToDwarf(modifier_current, key_current);
        }
        key_executed = true;
    }
    switch (wparam) {
    case VK_SHIFT:
        modifier_current &= ~KEY_SHIFT;
        break;
    case VK_CONTROL:
        modifier_current &= ~KEY_CTRL;
        break;
    case VK_MENU:
        modifier_current &= ~KEY_ALT;
        break;
    case VK_LWIN:
    case VK_RWIN:
        modifier_current &= ~KEY_META;
        break;
    case VK_UP:
        modifier_current &= ~KEY_UP;
        break;
    case VK_DOWN:
        modifier_current &= ~KEY_DOWN;
        break;
    case VK_LEFT:
        modifier_current &= ~KEY_LEFT;
        break;
    case VK_RIGHT:
        modifier_current &= ~KEY_RIGHT;
        break;
    default:
        if (key_current == wparam) {
            key_current = 0;
        }
        break;
    }
    if (modifier_current == 0 && key_current == 0) {
        key_executed = false;
        KillTimer(window, KEYSTROKE_REPEAT_TIMER);
    }
}

void keyDown(HWND window, WPARAM wparam, LPARAM lparam) {
    if (!(lparam & PREVIOUSLY_DOWN)) { // ignore system repeating
        switch (wparam) {
        case VK_SHIFT:
            modifier_current |= KEY_SHIFT;
            break;
        case VK_CONTROL:
            modifier_current |= KEY_CTRL;
            break;
        case VK_MENU:
            modifier_current |= KEY_ALT;
            break;
        case VK_LWIN:
        case VK_RWIN:
            modifier_current |= KEY_META;
            break;
        case VK_UP:
            modifier_current |= KEY_UP;
            break;
        case VK_DOWN:
            modifier_current |= KEY_DOWN;
            break;
        case VK_LEFT:
            modifier_current |= KEY_LEFT;
            break;
        case VK_RIGHT:
            modifier_current |= KEY_RIGHT;
            break;
        default:
            key_current = wparam;
            break;
        }
        key_executed = false;
        SetTimer(window, KEYSTROKE_REPEAT_TIMER, 350, keyRepeat);   // (re)set a repeat timer
    }
}

VOID CALLBACK keyRepeat(HWND window, UINT, UINT, DWORD) {
    if (!keyStroke(window, modifier_current, key_current)) {
        SendKeyToDwarf(modifier_current, key_current);
    }
    SetTimer(window, KEYSTROKE_REPEAT_TIMER, 100, keyRepeat);
    key_executed = true;
}

#endif

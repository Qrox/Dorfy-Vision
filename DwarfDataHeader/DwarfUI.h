#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_UI_H__
#define __DORFY_VISION__DWARF_UI_H__

#include <string>
#include "DwarfDataGeneral.h"

using namespace std;

static_assert(sizeof(void *) == 4, "please compile under 32 bit architecture!");

namespace DwarfFortress {
    namespace UI {
        enum UItype {
            unknown,
            announcement,
            dwarf_fortress,
            main_menu,
            options,
            keybinding,
            loading,
        };

        union Base {
            __at(0, VTable * vtable);
            __at(4, Base * next);
            __at(8, Base * previous);

            UItype getUItype();
        };

        union Announcement {
            union Data {
                __at(4, String text);
                __at(20, word color);
                __at(22, byte bright);
                __at(2C, dword repeat);
                __at(30, Vector3<int16> location);
                __at(3C, dword year);
                __at(40, dword ticks);
            };

            __at(4, Base * next);
            __at(8, Base * previous);
            __at(18, dword selection);
            __at(1C, Set<Data *> announcements);
        };

        union Adventurer {
            __at(4, Base * next);
            __at(8, Base * previous);
        };

        union Fortress {
            enum _ {
                none = 0,
                squad,
                stockpile = 15,
                build,
                building_task,
                building_items = 23,
                unit,
                look_around,
                hotkey = 37,
                depot_access = 48,
                point,
                route,
                burrow,
                hauling,
            };

            struct designation { enum {
                mine = 2,
                remove_up_ramp_stair,
                up_stair,
                down_stair,
                up_down_stair,
                up_ramp,
                channel,
                gather_plant,
                remove_designation,
                smooth_stone,
                carve_track,
                engrave_stone,
                carve_fortification,
                chop_tree = 34,
                toggle_engraving,
                toggle_standard_marking,
                remove_construction = 47, };

                struct mass { enum {
                    reclaim = 26,
                    forbid,
                    melt,
                    remove_melt,
                    dump,
                    remove_dump,
                    hide,
                    remove_hide,
                };};

                struct traffic { enum {
                    high = 38,
                    normal,
                    low,
                    restricted,
                };};
            };

            struct standing_order { enum {
                general = 18,
                forbid,
                refuse,
                workshop,
                zone,
            };};

            struct zone { enum {
                designation = 42,
                pen_pasture,
                pit_pond,
                hospital,
                gather,
            };};

            struct debug { enum {
                weather = 53,
                tree,
            };};

            __at(4, Base * next);
            __at(8, Base * previous);
        };

        union MainMenu {
            enum Mode : word {
                main = 0,
                start = 2,
                arena = 3,
                about = 4,
            };

            __at(4, Base * next);
            __at(8, Base * previous);
            __at(10, char splash_text[256]);
            __at(210, Mode mode);
            __at(214, dword selection);
            __at(218, dword arena_selection);
            __at(220, Set<dword> entries);
            __at(270, String title);
            __at(28C, String subtitle);
            __at(2A8, String copyright);
            __at(2C4, String version);
            __at(2E0, String top_text); // normally empty, flashes at the first line if not
            __at(2FC, String credit_1);
            __at(318, String credit_2);
            __at(334, String website_1);
            __at(350, String website_2);
        };

        union Options {
            __at(4, Base * next);
            __at(8, Base * previous);
            __at(18, dword selection);
            __at(1C, Set<dword> entries);
        };

        union Keybinding {
            __at(4, Base * next);
            __at(8, Base * previous);
            __at(1C, dword main_entry_count);
            __at(24, dword main_selection);
            __at(48, dword key_entry_count);
            __at(50, dword key_selection);
            __at(74, dword key_setting_count);
            __at(7C, dword key_setting_selection);
            __at(CC, dword binding_entry_count);
            __at(D4, dword binding_selection);
        };

        namespace SaveType {
            enum Enum : word {
                dwarf_fortress      = 0,
                adventurer          = 1,
                reclaim_fortress    = 3,
            };
        }

        union Loading {
            union SaveInfo {
                __at(80, word save_type);
                __at(84, String fortress_name);
                __at(84, String adventurer_name);
                __at(A0, String world_name);
                __at(BC, dword year);
                __at(C0, String folder_name);

                string const & saveTypeString();
            };

            __at(4, Base * next);
            __at(8, Base * previous);
            __at(1C, dword selection);
            __at(20, Set<SaveInfo *> save_info);
        };
    }
}

#endif

#endif

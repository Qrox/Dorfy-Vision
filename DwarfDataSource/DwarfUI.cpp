#ifdef BUILD_DLL

#include "DwarfFortress.h"
#include "DwarfUI.h"

namespace DwarfFortress {
    namespace UI {
        UItype Base::getUItype() {
            if (vtable == &df.vtables.ui.options) {
                return options;
            } else if (vtable == &df.vtables.ui.loading) {
                return loading;
            } else if (vtable == &df.vtables.ui.dwarf_fortress) {
                return dwarf_fortress;
            } else if (vtable == &df.vtables.ui.main_menu) {
                return main_menu;
            } else if (vtable == &df.vtables.ui.announcement) {
                return announcement;
            } else if (vtable == &df.vtables.ui.keybinding) {
                return keybinding;
            } else {
                return unknown;
            }
        }

        string const & Loading::SaveInfo::saveTypeString() {
            static string const names[] = {
                "Dwarf Fortress",
                "Adventurer",
                "Unknown Type Error",
                "Reclaim Fortress",
            };
            return names[save_type < 4 ? save_type : 2];
        }
    }
}

#endif

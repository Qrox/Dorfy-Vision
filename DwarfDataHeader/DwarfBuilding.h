#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_BUILDING_H__
#define __DORFY_VISION__DWARF_BUILDING_H__

#include "DwarfDataGeneral.h"

namespace DwarfFortress {
    union Building {
        union Content {

        };

        __at(0, union {
            __at(0, dword (__thiscall * getCustomType)(Building *));
            __at(4, void (__thiscall * setCustomType)(Building *, dword type));
        } * vtable);

        __at(4, dword x_start);
        __at(8, dword y_start);
        __at(C, dword work_location_x);
        __at(10, dword x_end);
        __at(14, dword y_end);
        __at(18, dword work_location_y);
        __at(1C, dword z);
        __at(48, dword index);
        __at(F4, word construction_progress);
        __at(F8, Set<Content *> contents);
        __at(10C, word workshop_type);
        __at(11E, word furnace_type);
        __at(170, word custom_workshop_type);
        __at(17C, word custom_furnace_type);
    };
}

#endif

#endif

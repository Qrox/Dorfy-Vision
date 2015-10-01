#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_DESCRIPTOR_H__
#define __DORFY_VISION__DWARF_DESCRIPTOR_H__

#include "DwarfDataGeneral.h"

namespace DwarfFortress {
    union ColorDescriptor {
        struct Color3f {
            float32 red, green, blue;
        };
        __at(0, String identifier);
        __at(3C, String name);
        __at(58, byte console_color);
        __at(59, byte console_color_bright);
        __at(5C, Color3f rgb);
    };
}

#endif

#endif

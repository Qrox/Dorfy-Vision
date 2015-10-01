#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_BIOME_H__
#define __DORFY_VISION__DWARF_BIOME_H__

#include "DwarfDataGeneral.h"

namespace DwarfFortress {
    union Biome {
        union Layer {
            __at(4, dword layer_inorganic);
        };

        __at(4, Set<Layer *> layers);
    };
}

#endif

#endif

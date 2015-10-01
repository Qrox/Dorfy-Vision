#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_REGION_BLOCK_H__
#define __DORFY_VISION__DWARF_REGION_BLOCK_H__

#include "DwarfDataGeneral.h"

namespace DwarfFortress {
    union NearbyRegionBlock {
        __at(12E4, word x);
        __at(12E6, word y);
        __at(32D4, word lava_stone_species);
    };

    union RegionBlock {
        __at(58, word biome_index);
        __at(5A, word unknown_5A);
    };
}

#endif

#endif

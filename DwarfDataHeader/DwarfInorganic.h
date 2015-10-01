#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_INORGANIC_H__
#define __DORFY_VISION__DWARF_INORGANIC_H__

#include "DwarfDataGeneral.h"
#include "DwarfMaterial.h"

namespace DwarfFortress {
    namespace EnvironmentClass {
        enum Enum : word {
            soil,
            soil_ocean,
            soil_sand,
            metamorphic,
            sedimentary,
            igneous_intrusive,
            igneous_extrusive,
            alluvial
        };
    }

    namespace EnvironmentInclusion {
        enum Enum : word {
            vein = 1,
            cluster,
            cluster_small,
            cluster_one
        };
    }

    union InorganicRaw {
        __at(0, String identifier);
        __at(2C, struct {
            dword
            lava : 1,
            unknown_1 : 1,
            unknown_2 : 1,
            sedimentary : 1,
            sedimentary_ocean_shallow : 1,
            igneous_intrusive : 1,
            igneous_extrusive : 1,
            metamorphic : 1,
            deep_surface : 1, // slade
            unknown_9 : 1,
            aquifer : 1,
            soil : 1,
            soil_ocean : 1,
            soil_sand : 1,
            sedimentary_ocean_deep : 1,
            unknown_15 : 1,
            special : 1,
            soil_trivial : 1,
            deep_special : 1,
            devine : 1,
            unknown_20_24 : 5,
            wafers : 1;
        } * flags);
        __at(E8, Set<EnvironmentClass::Enum> environment_class);
        __at(F8, Set<EnvironmentInclusion::Enum> environment_inclusion);
        __at(108, Set<byte> environment_frequency);
        __at(11C, MaterialRaw material);
    };
}

#endif

#endif

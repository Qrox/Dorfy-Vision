#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_BLOCK_3D__
#define __DORFY_VISION__DWARF_BLOCK_3D__

#include "DwarfDataGeneral.h"
#include "DwarfMaterial.h"
#include "DwarfTile.h"
#include "DwarfVegetation.h"

namespace DwarfFortress {
    union Block2d {
        __at(E44, Set<Vegetation *> vegetations);
    };

    struct TileFeature {
        enum Type : int8 {
            unknown = -1,
            mineral = 0,
            contaminant = 3,
            grass = 4,
        };

        struct {
            Type (__thiscall * type)(TileFeature *);
        } * vtable;

//        inline Type type() {
//            return vtable->type(this);
//        }
    };

    struct Contaminant : public TileFeature {
        dword material;
        dword species;
        word state;
        byte size[16][16];
    };

    struct Grass : public TileFeature {
        dword species;
        byte density[16][16];
    };

    struct Mineral : public TileFeature {
        dword species;
        word exists[16];
    };

    union Block3d {
        __at(4, Set<TileFeature *> tile_features);
        __at(38, Set<dword> items);
        __at(60, Vector3<int16> location);
        __at(6A, TileStructure::Enum tile_structure[16][16]);
        __at(26C, TileInfo tile_info[16][16]);
        __at(66C, Occupancy occupancy[16][16]);
        __at(156C, word temperature_1[16][16]);
        __at(176C, word temperature_2[16][16]);
        __at(1D6C, byte host_region_block_directions[16]);
    };
}

#endif

#endif

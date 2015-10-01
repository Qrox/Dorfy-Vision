#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_FORTRESS_H__
#define __DORFY_VISION__DWARF_FORTRESS_H__

#include <windows.h>
#include <string>
#include "DwarfDataGeneral.h"
#include "DwarfBiome.h"
#include "DwarfRegionBlock.h"
#include "DwarfBlock.h"
#include "DwarfItem.h"
#include "DwarfInorganic.h"
#include "DwarfCreature.h"
#include "DwarfLanguage.h"
#include "DwarfDescriptor.h"
#include "DwarfUI.h"

using namespace std;

namespace DwarfFortress {
    union World {
        __at(94, dword region_block_dimension_x);
        __at(98, dword region_block_dimension_y);
        __at(CC, Set<NearbyRegionBlock *> nearby_region_blocks);
        __at(1C0, Set<Biome *> biomes);
        __at(1F0, RegionBlock ** region_blocks);
    };

    union Map {
        //__at(C0, Set<Construction *> constructions);
        __at(13A64, Set<Creature *> creatures);
        __at(13B18, Set<Item *> items);
        //__at(14390, Set<Building *> buildings);
        __at(11286C, Block3d **** block3d);
        __at(112880, Block2d *** block2d);
        __at(112890, Vector3<dword> dimension);
        __at(112890, dword region_offset_x);
        __at(1128A0, dword region_offset_y);
        __at(115798, World * world);

        __at(115FFC, Set<InorganicRaw *> inorganic_raw);
        __at(11601C, Set<VegetationRaw *> vegetation_raw);
        __at(1160DC, Set<CreatureRaw *> creature_raw_by_name);
        __at(1160EC, Set<CreatureRaw *> creature_raw);
        __at(116350, Set<LanguageRaw *> language_raw);
        __at(116360, Set<LanguageSymRaw *> language_sym_raw);
        __at(116370, Set<RaceLanguageRaw *> race_language_raw);
        __at(11BD80, Set<ColorDescriptor *> descriptor_color);

        Block3d & getBlock3d(dword tile_x, dword tile_y, dword tile_z);
        Block2d & getBlock2d(dword tile_x, dword tile_y);
        bool getRegionBlockCoord(dword x, dword y, dword z, dword & rbx, dword & rby);
        NearbyRegionBlock & getNearbyRegionBlock(dword x, dword y, dword z);
        Biome & getBiome(dword x, dword y, dword z);
        Biome::Layer & getLayer(dword x, dword y, dword z);
        TileStructure::Enum tileStructure(dword x, dword y, dword z);
        TileInfo tileInfo(dword x, dword y, dword z);
        TileDefinition tileDefinition(dword x, dword y, dword z);
        int32 getLayerSpecies(dword x, dword y, dword z);
        int32 getMineralSpecies(dword x, dword y, dword z);
        int32 getLavaStoneSpecies(dword x, dword y, dword z);
        void getGrass(dword x, dword y, dword z, byte & density, int32 & species);
        void getContaminant(dword x, dword y, dword z, byte & size, int16 & material, int32 & species);
    };

    union Game {
        __at(0, union {
        } code);
        __at(97D000, union { // end of code, start of read-only memory
            union {
            } item;
            union {
                __at(2F54C, VTable options);
                __at(2F5A4, VTable loading);
                __at(38A44, VTable dwarf_fortress);
                __at(4BA9C, VTable main_menu);
                __at(4BB20, VTable announcement);
                __at(83224, VTable keybinding);
            } ui;
        } vtables);
        __at(A80000, union { // end of read-only memory, start of static memory
            __at(DB0, Vector3<int32> cursor);
            __at(E88, byte side_menu_pos);
            __at(E89, byte side_map_pos);
            __at(1654, byte fps_on);
            __at(165C, int32 view_width);
            __at(1660, int32 view_height);
            // used for :
            //     date calculation in the stock menu
            //     time progression
            // range 0 ~ 10079, increases by 1 every time df.tick reaches a multiple of 10
            // once reaches 10080, season is increased by 1 and season_time resets.
            __at(2E56E0, dword season_time);
            // used for :
            //     date calculation in the stock menu
            //     time progression
            // range 0 ~ 3, corresponding spring ~ winter
            __at(2E5806, byte season);
            __at(37DEF8, int32 view_x);
            // used for :
            //     date calculation in the announcement & the report menus
            //     not used for time progression
            // range 0 ~ 403199
            __at(3926D8, dword ticks);
            __at(3AC0E4, int32 view_z);
            __at(3AC108, dword look_menu_index);
            __at(3AC110, int32 view_y);
            __at(3C4828, dword year);
            __at(9BBE28, dword ticks_per_sec);
            __at(9BBE2C, dword frames_per_sec);
            __at(9BBF14, float32 console_color_rgb[16][3]); // [bright:r:g:b]
            __at(9C3968, word side_menu_ui);
            __at(9C3A60, UI::Base * ui_bottom);
            __at(B7E440, Map map);
        });

        UI::Base & getTopmostUI();
        dword getFortressModeViewWidth();
        dword getFortressModeViewHeight();

        string const & seasonName();
        dword month();
        string const & dwarvenMonthName();
        dword day();
        string dayName();

        static float const * getHardCodedMaterialColor(int32 material);
        float const * getConsoleColor(byte console_color, byte console_color_bright);
        float const * checkFineColor(int32 index, byte console_color, byte console_color_bright);
        MaterialRaw & getMaterial(int16 material, int32 species);
        bool isMetal(int16 material, int32 species);
        bool isGlass(int16 material, int32 species);
        bool isMetalOrGlass(int16 material, int32 species);
        float const * getColorOfMaterial(int16 material, int32 species, MaterialState::Enum state, TemplateColorType::Enum type, bool console_strict);
        float const * getColorOfMaterial(int16 material, int32 species, MaterialState::Enum state, byte console_color, byte console_color_bright, bool console_strict);
    };

    namespace ConsoleColor {
        enum Enum {
            black,
            blue,
            green,
            cyan,
            red,
            magenta,
            brown,
            light_gray,

            dark_gray,
            light_blue,
            light_green,
            light_cyan,
            light_red,
            light_magenta,
            yellow,
            white,
        };
    }
}

extern DwarfFortress::Game * pDF;
extern HMODULE dfModule;
extern HWND dfWindow;

#define df (*pDF)

#endif

#endif

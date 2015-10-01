#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_VEGETATION_H__
#define __DORFY_VISION__DWARF_VEGETATION_H__

#include "DwarfDataGeneral.h"
#include "DwarfMaterial.h"

namespace DwarfFortress {
    union VegetationRaw {
        union GrowthPrint {
            __at(0, dword priority);
            __at(4, byte char_tile);
            __at(5, byte char_item);
            __at(6, word color_fore);
            __at(8, word color_back);
            __at(A, word color_fore_bright);
            __at(C, int32 time_start);
            __at(10, int32 time_end);
        };

        union GrowthRaw {
            __at(0, String identifier);
            __at(1C, String name_singular);
            __at(38, String name_plural);
            __at(E4, int16 material);
            __at(E8, int32 species);
            __at(EC, Set<GrowthPrint *> print);
            __at(FC, int32 time_start);
            __at(100, int32 time_end);
            __at(104, struct {
                dword
                twigs : 1,
                branches : 1,
                heavy_branches : 1,
                trunk : 1,
                roots : 1,
                cap : 1,
                sapling : 1;
            } host_tile);
            __at(108, dword density);
        };

        __at(0, String identifier);
        __at(1C, dword index);
        __at(30, struct {
            dword
            spring : 1,
            summer : 1,
            autumn : 1,
            winter : 1,
            unknown_4_19 : 16,
            wet : 1,
            dry : 1,
            mountain : 1,
            glacier : 1,
            tundra : 1,
            swamp_temperate_freshwater : 1,
            swamp_temperate_saltwater : 1,
            marsh_temperate_freshwater : 1,
            marsh_temperate_saltwater : 1,
            swamp_tropical_freshwater : 1,
            swamp_tropical_saltwater : 1,
            swamp_mangrove : 1,

            marsh_tropical_freshwater : 1,
            marsh_tropical_saltwater : 1,
            forest_taiga : 1,
            forest_temperate_conifer : 1,
            forest_temperate_broadleaf : 1,
            forest_tropical_conifer : 1,
            forest_tropical_dry_broadleaf : 1,
            forest_tropical_moist_broadleaf : 1,
            grassland_temperate : 1,
            savanna_temperate : 1,
            shrubland_temperate : 1,
            grassland_tropical : 1,
            savanna_tropical : 1,
            shrubland_tropical : 1,
            desert_badland : 1,
            desert_rock : 1,
            desert_sand : 1,
            ocean_tropical : 1,
            ocean_temperate : 1,
            ocean_arctic : 1,
            pool_temperate_freshwater : 1,
            unknown_53_55 : 3,
            good : 1,
            evil : 1,
            savage : 1,
            pool_temperate_brackishwater : 1,
            pool_temperate_saltwater : 1,
            pool_tropical_freshwater : 1,
            pool_tropical_brackishwater : 1,
            pool_tropical_saltwater : 1,

            lake_temperate_freshwater : 1,
            lake_temperate_brackishwater : 1,
            lake_temperate_saltwater : 1,
            lake_tropical_freshwater : 1,
            lake_tropical_brackishwater : 1,
            lake_tropical_saltwater : 1,
            river_temperate_freshwater : 1,
            river_temperate_brackishwater : 1,
            river_temperate_saltwater : 1,
            river_tropical_freshwater : 1,
            river_tropical_brackishwater : 1,
            river_tropical_saltwater : 1,
            subterranean_water : 1,
            subterranean_chasm : 1,
            subterranean_lava : 1;
        } * environment);
        __at(38, String name_singular);
        __at(54, String name_plural);
        __at(70, String adjective);
        __at(8C, String seed_name_singular);
        __at(A8, String seed_name_plural);
        __at(190, dword grow_duration);
        __at(194, dword plant_value);
        __at(198, byte picked_color_fore);
        __at(199, byte picked_color_back);
        __at(19A, byte picked_color_fore_bright);
        __at(19B, byte dead_picked_color_fore);
        __at(19C, byte dead_picked_color_back);
        __at(19D, byte dead_picked_color_fore_bright);
        __at(19E, byte shrub_color_fore);
        __at(19F, byte shrub_color_back);
        __at(1A0, byte shrub_color_fore_bright);
        __at(1A1, byte dead_shrub_color_fore);
        __at(1A2, byte dead_shrub_color_back);
        __at(1A3, byte dead_shrub_color_fore_bright);
        __at(1A4, byte seed_color_fore);
        __at(1A5, byte seed_color_back);
        __at(1A6, byte seed_color_fore_bright);
        __at(1A7, byte tree_color_fore);
        __at(1A8, byte tree_color_back);
        __at(1A9, byte tree_color_fore_bright);
        __at(1AA, byte dead_tree_color_fore);
        __at(1AB, byte dead_tree_color_back);
        __at(1AC, byte dead_tree_color_fore_bright);
        __at(1AD, byte sapling_color_fore);
        __at(1AE, byte sapling_color_back);
        __at(1AF, byte sapling_color_fore_bright);
        __at(1B0, byte dead_sapling_color_fore);
        __at(1B1, byte dead_sapling_color_back);
        __at(1B2, byte dead_sapling_color_fore_bright);
        __at(1C3, byte grass_color_1_fore);
        __at(1C4, byte grass_color_2_fore);
        __at(1C5, byte grass_color_dry_fore);
        __at(1C6, byte grass_color_dead_fore);
        __at(1D7, byte grass_color_1_back);
        __at(1D8, byte grass_color_2_back);
        __at(1D9, byte grass_color_dry_back);
        __at(1DA, byte grass_color_dead_back);
        __at(1EB, byte grass_color_1_fore_bright);
        __at(1EC, byte grass_color_2_fore_bright);
        __at(1ED, byte grass_color_dry_fore_bright);
        __at(1EE, byte grass_color_dead_fore_bright);
        __at(1F0, dword grass_alt_period_1);
        __at(1F4, dword grass_alt_period_2);
        __at(1F8, byte shrub_drown_level);
        __at(1F9, byte tree_drown_level);
        __at(1FA, byte sapling_drown_level);
        __at(1FC, word frequency);
        __at(1FE, word cluster_size);
        __at(200, Set<String *> pref_string);
        __at(210, Set<MaterialRaw *> materials);
        __at(220, int16 basic_material);
        __at(222, int16 tree_material);
        __at(224, int16 drink_material);
        __at(226, int16 seed_material);
        __at(228, int16 thread_material);
        __at(22A, int16 mill_material);
        __at(234, int32 basic_species);
        __at(238, int32 tree_species);
        __at(23C, int32 drink_species);
        __at(240, int32 seed_species);
        __at(244, int32 thread_species);
        __at(248, int32 mill_species);
        __at(54C, dword underground_depth_min);
        __at(550, dword underground_depth_max);
        __at(554, Set<GrowthRaw *> growth);
        __at(564, String root_name);
        __at(580, String trunk_name);
        __at(59C, String thick_branch_name);
        __at(5B8, String branch_name);
        __at(5D4, String twig_name);
        __at(5F0, String cap_name);
        __at(60C, dword trunk_period);
        __at(610, dword heavy_branch_density);
        __at(614, dword branch_density);
        __at(618, dword max_trunk_height);
        __at(61C, dword heavy_branch_radius);
        __at(620, dword branch_radius);
        __at(624, dword trunk_branching);
        __at(628, dword max_trunk_diameter);
        __at(62C, dword trunk_width_period);
        __at(630, dword cap_period);
        __at(634, dword cap_radius);
        __at(638, dword root_density);
        __at(63C, dword root_radius);
    };

    union Vegetation {
        union Dimension {
            __at(0, byte ** body_occupancy); // [pos_z][pos_x + pos_y * size_y], 0 = none, 0x80 = others, else = self
            __at(14, dword z);
            __at(18, dword x);
            __at(1C, dword y);
            __at(20, byte ** root_occupancy); // [pos_z][pos_x + pos_y * size_y], 0 = none, 0x80 = others, else = self
            __at(24, dword root_depth);
        };

        __at(0, word naming);
        __at(2, word species);
        __at(4, Vector3<int16> location);
        //__at(C, dword some_time);
        __at(34, Dimension * dimension);

        static dword getGrowthTicksOffset(int32 x, int32 y);
        static dword getGrowthTicks(int32 x, int32 y);
        dword getGrowthTicks();
    };
}

#endif

#endif

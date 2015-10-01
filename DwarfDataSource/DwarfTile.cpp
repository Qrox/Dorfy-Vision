#ifdef BUILD_DLL

#include <algorithm>
#include "DwarfTile.h"

using namespace std;

namespace DwarfFortress {

    static TileDefinition d[TileStructure::structure_end];

    void tileDefinitionInit() {
        dword tmp;
        #define declare(v) tmp = v;
        {
            using namespace BodyType;
            #define _(type) d[TileStructure::type].bodytype = tmp;
            #define __(from, to) for (dword index = TileStructure::from; index <= TileStructure::to; ++index) d[index].bodytype = tmp;
            declare(none)
            _(murky_pool)
            _(open_space)
            _(chasm)
            _(eerie_glowing_pit)
            __(smooth_layer_floor, level_ice_floor)
            _(ashes_floor_burning_1)
            _(ashes_floor_burning_2)
            __(ashes_floor_burnt_1, ashes_floor_burnt_3)
            __(ice_floor_1, ice_floor_4)
            _(lava_flow)
            _(glowing_floor)
            __(layer_floor_1, wet_soil_floor_4)
            __(river_1, brook_8)
            __(dry_grass_floor_1, dry_grass_floor_4)
            __(dead_grass_floor_1, dead_grass_floor_4)
            __(mineral_floor_1, mineral_floor_4)
            _(constructed_floor)
            declare(wall)
            __(layer_pillar, ice_pillar)
            __(layer_wall_mined_3, layer_wall_mined_1)
            _(layer_wall)
            _(semi_molten_rock)
            _(soil_wall)
            _(glowing_barrier)
            __(smooth_obsidian_wall_Se, smooth_layer_wall_WE)
            __(obsidian_wall_mined_3, unknown_wall)
            __(ice_wall_mined_3, ice_wall)
            __(smooth_mineral_wall_Se, smooth_mineral_wall_WE)
            __(mineral_wall_mined_3, mineral_wall)
            __(straight_ice_wall_Se, straight_ice_wall_WE)
            __(constructed_pillar, constructed_wall_WE)
            declare(fortification)
            _(layer_fortification)
            _(obsidian_fortification)
            _(unknown_fortification)
            _(ice_fortification)
            _(mineral_fortification)
            _(constructed_fortification)
            declare(rampup_trivial)
            _(murky_pool_slope_up)
            __(dry_grass_slope_up, soil_slope_up)
            _(glacial_slope_up)
            __(river_slope_up_1, river_slope_up_8)
            _(constructed_slope_up)
            declare(rampdown)
            _(downward_slope)
            declare(stairboth)
            _(underworld_gate_both)
            _(ice_stair_both)
            _(obsidian_stair_both)
            _(soil_stair_both)
            _(grass_2_stair_both)
            _(grass_1_stair_both)
            _(layer_stair_both)
            _(mineral_stair_both)
            _(unknown_stair_both)
            _(constructed_stair_both)
            declare(stairdown)
            _(underworld_gate_down)
            _(ice_stair_down)
            _(obsidian_stair_down)
            _(soil_stair_down)
            _(grass_2_stair_down)
            _(grass_1_stair_down)
            _(layer_stair_down)
            _(mineral_stair_down)
            _(unknown_stair_down)
            _(constructed_stair_down)
            declare(stairup)
            _(underworld_gate_up)
            _(ice_stair_up)
            _(obsidian_stair_up)
            _(soil_stair_up)
            _(grass_2_stair_up)
            _(grass_1_stair_up)
            _(layer_stair_up)
            _(mineral_stair_up)
            _(unknown_stair_up)
            _(constructed_stair_up)
            declare(track)
            __(layer_track_N, constructed_track_NSEW)
            declare(trackup)
            __(layer_track_up_N, layer_track_up_NSEW)
            declare(shrub)
            _(shrub)
            _(dead_shrub)
            declare(sapling)
            _(sapling)
            _(dead_sapling)
            declare(tree_trivial)
            __(tree_root_vertical_end, dead_tree_thick_branches_we)
            __(dead_tree_thick_branches_nw, dead_cap_dot_4)
            __(tree_thick_branches_nse, dead_tree_trunk_pith)
            declare(tree_rampup)
            _(cap_slope_up)
            _(dead_cap_slope_up)
            declare(pebble)
            __(layer_pebbles_1, unknown_pebbles_4)
            __(mineral_pebbles_1, mineral_pebbles_4)
            declare(boulder)
            __(layer_boulder, unknown_boulder)
            _(mineral_boulder)
            #undef __
            #undef _
            #define _(type) d[TileStructure::type].hasbase = true; d[TileStructure::type].hasfloor = true;
            #define __(from, to) for (dword index = TileStructure::from; index <= TileStructure::to; ++index) {d[index].hasbase = true; d[index].hasfloor = true;}
            _(murky_pool)
            __(smooth_layer_floor, level_ice_floor)
            _(ashes_floor_burning_1)
            _(ashes_floor_burning_2)
            __(ashes_floor_burnt_1, ashes_floor_burnt_3)
            __(ice_floor_1, ice_floor_4)
            _(lava_flow)
            _(glowing_floor)
            __(layer_floor_1, wet_soil_floor_4)
            __(river_1, brook_8)
            __(dry_grass_floor_1, dry_grass_floor_4)
            __(dead_grass_floor_1, grass_2_floor_4)
            __(mineral_floor_1, mineral_floor_4)
            _(constructed_floor)
            for (dword index = 0; index < TileStructure::structure_end; ++index) {
                dword def = d[index].bodytype;
                if (def != none         &&
                    def != rampdown     &&
                    def != tree_trivial &&
                    def != tree_rampup  ) {
                    d[index].hasbase = true;
                }
                if (def != none         &&
                    def != rampdown     &&
                    def != stairboth    &&
                    def != stairdown    &&
                    def != tree_trivial &&
                    def != tree_rampup  ) {
                    d[index].hasfloor = true;
                }
                if (def == wall          ||
                    def == fortification ||
                    def == tree_trivial  ) {
                    d[index].isblocked = true;
                }
                if (def == rampup_trivial ||
                    def == tree_rampup) {
                    d[index].isupslope = true;
                }
            }
        }
        {
            using namespace TreeType;
            #undef __
            #undef _
            #define _(type) d[TileStructure::type].isalive = true;
            #define __(from, to) for (dword index = TileStructure::from; index <= TileStructure::to; ++index) d[index].isalive = true;
            __(tree_root_vertical_end, tree_branches_smooth)
            __(tree_thick_branches_nw, cap_pointed_4)
            __(tree_thick_branches_nse, tree_thick_branches_nswe)
            __(tree_trunk_NSE, tree_trunk_pith)
            _(sapling)
            _(shrub)
            __(grass_2_stair_both, grass_2_stair_up)
            __(grass_1_stair_both, grass_1_stair_up)
            _(dry_grass_slope_up)
            _(grass_2_slope_up)
            _(grass_1_slope_up)
            __(grass_1_floor_1, grass_1_floor_4)
            __(dry_grass_floor_1, dry_grass_floor_4)
            __(grass_2_floor_1, grass_2_floor_4)
            #undef __
            #undef _
            #define _(type) d[TileStructure::type].treetype = tmp;
            #define __(from, to) for (dword index = TileStructure::from; index <= TileStructure::to; ++index) d[index].treetype = tmp;
            declare(root)
            _(tree_root_vertical_end)
            _(tree_root)
            _(dead_tree_root_vertical_end)
            _(dead_tree_root)
            declare(trunk)
            __(tree_trunk_pillar, tree_trunk_NSw)
            __(dead_tree_trunk_pillar, dead_tree_trunk_NSw)
            __(tree_trunk_NSE, dead_tree_trunk_pith)
            declare(thick_branches)
            _(tree_thick_branches_ns)
            _(tree_thick_branches_we)
            __(tree_thick_branches_nw, tree_thick_branches_se)
            _(dead_tree_thick_branches_ns)
            _(dead_tree_thick_branches_we)
            __(dead_tree_thick_branches_nw, dead_tree_thick_branches_se)
            __(tree_thick_branches_nse, dead_tree_thick_branches_nswe)
            declare(branches)
            _(tree_branches_smooth)
            _(dead_tree_branches_smooth)
            _(tree_branches)
            _(dead_tree_branches)
            declare(twigs)
            _(tree_twigs)
            _(dead_tree_twigs)
            declare(cap)
            __(cap_slope_up, cap_pointed_4)
            __(dead_cap_slope_up, dead_cap_dot_4)
        }
        {
            using namespace GrassType;
            #undef __
            #undef _
            #define _(type) d[TileStructure::type].grasstype = tmp;
            #define __(from, to) for (dword index = TileStructure::from; index <= TileStructure::to; ++index) d[index].grasstype = tmp;
            declare(normal_1)
            __(grass_1_stair_both, grass_1_stair_up)
            _(grass_1_slope_up)
            __(grass_1_floor_1, grass_1_floor_4)
            declare(normal_2)
            __(grass_2_stair_both, grass_2_stair_up)
            _(grass_2_slope_up)
            __(grass_2_floor_1, grass_2_floor_4)
            declare(dry)
            _(dry_grass_slope_up)
            __(dry_grass_floor_1, dry_grass_floor_4)
            declare(dead)
            _(dead_grass_slope_up)
            __(dead_grass_floor_1, dead_grass_floor_4)
        }
        {
            using namespace TileFeatureType;
            #undef __
            #undef _
            #define _(type) d[TileStructure::type].tilefeaturetype = tmp;
            #define __(from, to) for (dword index = TileStructure::from; index <= TileStructure::to; ++index) d[index].tilefeaturetype = tmp;
            declare(layer_rock);
            _(smooth_layer_floor)
            __(layer_stair_both, layer_stair_up)
            _(layer_fortification)
            _(layer_pillar)
            __(layer_wall_mined_3, layer_wall_mined_1)
            _(layer_wall)
            _(layer_slope_up)
            __(smooth_layer_wall_Se, smooth_layer_wall_WE)
            __(layer_floor_1, layer_floor_2)
            _(layer_boulder)
            __(layer_pebbles_1, layer_pebbles_4)
            __(layer_track_N, layer_track_NSEW)
            __(layer_track_up_N, layer_track_up_NSEW)
            declare(layer_soil)
            _(shrub)
            __(soil_stair_both, soil_stair_up)
            _(sapling)
            _(soil_slope_up)
            _(furrowed_soil_floor)
            _(soil_wall)
            __(soil_floor_1, soil_floor_4)
            __(wet_soil_floor_1, wet_soil_floor_4)
            _(dead_sapling)
            _(dead_shrub)
            declare(grass)
            __(grass_2_stair_both, grass_2_stair_up)
            __(grass_1_stair_both, grass_1_stair_up)
            _(dry_grass_slope_up)
            _(dead_grass_slope_up)
            _(grass_2_slope_up)
            _(grass_1_slope_up)
            __(grass_1_floor_1, grass_1_floor_4)
            __(dry_grass_floor_1, dry_grass_floor_4)
            __(dead_grass_floor_1, dead_grass_floor_4)
            __(grass_2_floor_1, grass_2_floor_4)
            declare(mineral)
            _(smooth_mineral_floor)
            __(mineral_stair_both, mineral_stair_up)
            _(mineral_pillar)
            _(mineral_slope_up)
            __(smooth_mineral_wall_Se, smooth_mineral_wall_WE)
            _(mineral_fortification)
            __(mineral_wall_mined_3, mineral_wall)
            __(mineral_floor_1, mineral_floor_4)
            _(mineral_boulder)
            __(mineral_pebbles_1, mineral_pebbles_4)
            __(mineral_track_N, mineral_track_NSEW)
            __(mineral_track_up_N, mineral_track_up_NSEW)
            declare(unknown)
            _(smooth_unknown_floor)
            __(unknown_stair_both, unknown_stair_up)
            _(unknown_pillar)
            _(unknown_slope_up)
            __(smooth_unknown_wall_Se, smooth_unknown_wall_WE)
            _(unknown_fortification)
            __(unknown_wall_mined_3, unknown_wall)
            __(unknown_floor_1, unknown_floor_4)
            _(unknown_boulder)
            __(unknown_pebbles_1, unknown_pebbles_4)
            __(unknown_track_N, unknown_track_NSEW)
            __(unknown_track_up_N, unknown_track_up_NSEW)
            declare(obsidian)
            __(obsidian_stair_both, obsidian_stair_up)
            _(smooth_obsidian_floor)
            _(obsidian_pillar)
            _(obsidian_slope_up)
            __(smooth_obsidian_wall_Se, smooth_obsidian_wall_WE)
            _(obsidian_fortification)
            __(obsidian_wall_mined_3, obsidian_wall)
            __(obsidian_floor_1, obsidian_floor_4)
            _(obsidian_boulder)
            __(obsidian_pebbles_1, obsidian_pebbles_4)
            __(obsidian_track_N, obsidian_track_NSEW)
            __(obsidian_track_up_N, obsidian_track_up_NSEW)
            declare(ice)
            __(ice_stair_both, ice_stair_up)
            _(level_ice_floor)
            _(ice_pillar)
            __(ice_floor_1, ice_floor_3)
            _(ice_floor_4)
            _(ice_fortification)
            __(ice_wall_mined_3, ice_wall)
            __(straight_ice_wall_Se, straight_ice_wall_WE)
            __(ice_track_N, ice_track_NSEW)
            __(ice_track_up_N, ice_track_up_NSEW)
        }
        {
            using namespace TemplateColorType;
            #undef __
            #undef _
            #define _(type) d[TileStructure::type].templatecolortype = tmp;
            #define __(from, to) for (dword index = TileStructure::from; index <= TileStructure::to; ++index) d[index].templatecolortype = tmp;
            declare(build)
            __(underworld_gate_up, underworld_gate_both)
            _(glowing_barrier)
            _(glowing_floor)
            declare(tile)
            __(layer_wall_mined_3, layer_wall_mined_1)
            _(layer_wall)
            _(soil_wall)
            __(obsidian_wall_mined_3, unknown_wall)
            __(ice_wall_mined_3, ice_wall)
            __(mineral_wall_mined_3, mineral_wall)
        }
        #undef declare
        #undef __
        #undef _
//        #define _(type) d[TileStructure::type].isfull = true;
//        #define __(from, to) for (dword index = TileStructure::from; index <= TileStructure::to; ++index) d[index].templatecolortype = tmp;
//        __(tree_branches, tree_twigs)
//        __(dead_tree_branches, dead_tree_twigs)
//        _(layer_wall)
//        _(soil_wall)
//        _(glowing_barrier)
//        __(smooth_obsidian_wall_Se, smooth_layer_wall_WE)
//        _(obsidian_wall)
//        _(unknown_wall)
//        _(ice_wall)
//        __(smooth_mineral_wall_Se, smooth_mineral_wall_WE)
//        _(mineral_wall)
//        __(straight_ice_wall_Se, straight_ice_wall_WE)
//        __(constructed_wall_Se, constructed_wall_WE)
//        #undef __
//        #undef _
        #define _(type, n, s, w, e)                                         \
            d[TileStructure::type].hasconnection = true;                    \
            d[TileStructure::type].connection.north = n;                    \
            d[TileStructure::type].connection.south = s;                    \
            d[TileStructure::type].connection.west = w;                     \
            d[TileStructure::type].connection.east = e;                     \
            d[TileStructure::dead_##type].hasconnection = true;             \
            d[TileStructure::dead_##type].connection.north = n;             \
            d[TileStructure::dead_##type].connection.south = s;             \
            d[TileStructure::dead_##type].connection.west = w;              \
            d[TileStructure::dead_##type].connection.east = e;
        _(tree_trunk_pith, 3, 3, 3, 3)
        _(tree_trunk_bark_N, 0, 3, 3, 3)
        _(tree_trunk_bark_S, 3, 0, 3, 3)
        _(tree_trunk_bark_E, 3, 3, 3, 0)
        _(tree_trunk_bark_W, 3, 3, 0, 3)
        _(cap_pith, 3, 3, 3, 3)
        _(cap_N, 0, 3, 3, 3)
        _(cap_S, 3, 0, 3, 3)
        _(cap_E, 3, 3, 3, 0)
        _(cap_W, 3, 3, 0, 3)
        _(cap_SE, 0, 3, 0, 3)
        _(cap_SW, 0, 3, 3, 0)
        _(cap_NE, 3, 0, 0, 3)
        _(cap_NW, 3, 0, 3, 0)
        _(tree_trunk_NSWE, 2, 2, 2, 2)
        _(tree_trunk_NSE, 2, 2, 0, 2)
        _(tree_trunk_NSW, 2, 2, 2, 0)
        _(tree_trunk_WEN, 2, 0, 2, 2)
        _(tree_trunk_WES, 0, 2, 2, 2)
        _(tree_trunk_SE, 0, 2, 0, 2)
        _(tree_trunk_SW, 0, 2, 2, 0)
        _(tree_trunk_NE, 2, 0, 0, 2)
        _(tree_trunk_NW, 2, 0, 2, 0)
        _(tree_trunk_NS, 2, 2, 0, 0)
        _(tree_trunk_WE, 0, 0, 2, 2)
        _(tree_trunk_WEn, 1, 0, 2, 2)
        _(tree_trunk_WEs, 0, 1, 2, 2)
        _(tree_trunk_NSe, 2, 2, 0, 1)
        _(tree_trunk_NSw, 2, 2, 1, 0)
        _(tree_thick_branches_nswe, 1, 1, 1, 1)
        _(tree_thick_branches_nse, 1, 1, 0, 1)
        _(tree_thick_branches_nsw, 1, 1, 1, 0)
        _(tree_thick_branches_wen, 1, 0, 1, 1)
        _(tree_thick_branches_wes, 0, 1, 1, 1)
        _(tree_thick_branches_ns, 1, 1, 0, 0)
        _(tree_thick_branches_we, 0, 0, 1, 1)
        _(tree_thick_branches_nw, 1, 0, 1, 0)
        _(tree_thick_branches_ne, 1, 0, 0, 1)
        _(tree_thick_branches_sw, 0, 1, 1, 0)
        _(tree_thick_branches_se, 0, 1, 0, 1)
        #undef _
        #define __(type, n, s, w, e)                                \
            d[TileStructure::type].hasconnection = true; \
            d[TileStructure::type].connection.north = n;            \
            d[TileStructure::type].connection.south = s;            \
            d[TileStructure::type].connection.west = w;             \
            d[TileStructure::type].connection.east = e;
        #define _(type, n, s, w, e)                                 \
                __(smooth_obsidian_wall_##type, n, s, w, e)         \
                __(smooth_unknown_wall_##type, n, s, w, e)          \
                __(smooth_layer_wall_##type, n, s, w, e)            \
                __(smooth_mineral_wall_##type, n, s, w, e)          \
                __(straight_ice_wall_##type, n, s, w, e)            \
                __(constructed_wall_##type, n, s, w, e)
        _(Se, 0, 2, 0, 1)
        _(sE, 0, 1, 0, 2)
        _(nE, 1, 0, 0, 2)
        _(Ne, 2, 0, 0, 1)
        _(nW, 1, 0, 2, 0)
        _(Nw, 2, 0, 1, 0)
        _(sW, 0, 1, 2, 0)
        _(Sw, 0, 2, 1, 0)
        _(NSWE, 2, 2, 2, 2)
        _(NSE, 2, 2, 0, 2)
        _(WES, 0, 2, 2, 2)
        _(WEN, 2, 0, 2, 2)
        _(NSW, 2, 2, 2, 0)
        _(SE, 0, 2, 0, 2)
        _(NE, 2, 0, 0, 2)
        _(NW, 2, 0, 2, 0)
        _(SW, 0, 2, 2, 0)
        _(NS, 2, 2, 0, 0)
        _(WE, 0, 0, 2, 2)
        #undef __
        #undef _
        #define __(type, n, s, w, e)                                \
            d[TileStructure::type].hasconnection = true; \
            d[TileStructure::type].connection.north = n;            \
            d[TileStructure::type].connection.south = s;            \
            d[TileStructure::type].connection.west = w;             \
            d[TileStructure::type].connection.east = e;
        #define _(type, n, s, w, e)                                 \
            __(layer_track_##type, n, s, w, e)                      \
            __(obsidian_track_##type, n, s, w, e)                   \
            __(unknown_track_##type, n, s, w, e)                    \
            __(mineral_track_##type, n, s, w, e)                    \
            __(ice_track_##type, n, s, w, e)                        \
            __(constructed_track_##type, n, s, w, e)                \
            __(layer_track_up_##type, n, s, w, e)                   \
            __(obsidian_track_up_##type, n, s, w, e)                \
            __(unknown_track_up_##type, n, s, w, e)                 \
            __(mineral_track_up_##type, n, s, w, e)                 \
            __(ice_track_up_##type, n, s, w, e)                     \
            __(constructed_track_up_##type, n, s, w, e)
        _(N, 1, 0, 0, 0)
        _(S, 0, 1, 0, 0)
        _(E, 0, 0, 0, 1)
        _(W, 0, 0, 1, 0)
        _(NS, 1, 1, 0, 0)
        _(NE, 1, 0, 0, 1)
        _(NW, 1, 0, 1, 0)
        _(SE, 0, 1, 0, 1)
        _(SW, 0, 1, 1, 0)
        _(EW, 0, 0, 1, 1)
        _(NSE, 1, 1, 0, 1)
        _(NSW, 1, 1, 1, 0)
        _(NEW, 1, 0, 1, 1)
        _(SEW, 0, 1, 1, 1)
        _(NSEW, 1, 1, 1, 1)
    }

    static TileDefinition constexpr nil = {0};
    TileDefinition const tileDefinition(TileStructure::Enum structure) {
        if (structure < TileStructure::structure_end) {
            return d[structure];
        } else {
            return nil;
        }
    }
}

#endif

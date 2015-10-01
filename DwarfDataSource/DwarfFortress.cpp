#ifdef BUILD_DLL

#include "Utils.h"
#include "DwarfFortress.h"
#include "DwarfTile.h"

namespace DwarfFortress {
    Block3d & Map::getBlock3d(dword tile_x, dword tile_y, dword tile_z) {
        Block3d *** block3dx, ** block3dxy;
        if (block3d &&
            tile_x >= 0 && tile_x < dimension.x &&
            tile_y >= 0 && tile_y < dimension.y &&
            tile_z >= 0 && tile_z < dimension.z &&
            (block3dx = block3d[tile_x / 16]) &&
            (block3dxy = block3dx[tile_y / 16])) {
            return *(block3dxy[tile_z]);
        } else {
            return *(Block3d *) nullptr;
        }
    }

    Block2d & Map::getBlock2d(dword tile_x, dword tile_y) {
        Block2d ** block2dx;
        if (block2d &&
            tile_x >= 0 && tile_x < dimension.x &&
            tile_y >= 0 && tile_y < dimension.y &&
            (block2dx = block2d[tile_x / 16])) {
            return *(block2dx[tile_y / 16]);
        } else {
            return *(Block2d *) nullptr;
        }
    }

    TileStructure::Enum Map::tileStructure(dword x, dword y, dword z) {
        Block3d & block = getBlock3d(x, y, z);
        if (&block) {
            return block.tile_structure[x & 15][y & 15];
        } else {
            return TileStructure::none;
        }
    }

    TileDefinition Map::tileDefinition(dword x, dword y, dword z) {
        return DwarfFortress::tileDefinition(tileStructure(x, y, z));
    }

    TileInfo Map::tileInfo(dword x, dword y, dword z) {
        Block3d & block = getBlock3d(x, y, z);
        if (&block) {
            return block.tile_info[x & 15][y & 15];
        } else {
            return {0};
        }
    }

    bool Map::getRegionBlockCoord(dword x, dword y, dword z, dword & rbx, dword & rby) {
        if (world && world->region_blocks) {
            Block3d & block = getBlock3d(x, y, z);
            if (&block) {
                dword local_x = x & 15, local_y = y & 15;
                dword region_block_bias = block.tile_info[local_x][local_y].region_block_bias;
                dword host_region_block_direction = block.host_region_block_directions[region_block_bias];
                int32 region_x_bias = host_region_block_direction % 3 - 1;
                int32 region_y_bias = host_region_block_direction / 3;
                if (region_y_bias >= 3) {
                    region_y_bias = 0; // to be consistent with the game binary (0 -> -1, 2 -> 1, else -> 0)
                } else {
                    --region_y_bias;
                }
                rbx = limit(0u, (region_offset_x + x / 48) / 16 + region_x_bias, world->region_block_dimension_x - 1);
                rby = limit(0u, (region_offset_y + y / 48) / 16 + region_y_bias, world->region_block_dimension_y - 1);
                return true;
            }
        }
        return false;
    }

    NearbyRegionBlock & Map::getNearbyRegionBlock(dword x, dword y, dword z) {
        dword region_block_x, region_block_y;
        if (getRegionBlockCoord(x, y, z, region_block_x, region_block_y)) {
            for (auto i = world->nearby_region_blocks.begin(); i < world->nearby_region_blocks.end(); ++i) {
                NearbyRegionBlock & b = **i;
                if (&b && b.x == region_block_x && b.y == region_block_y) {
                    return b;
                }
            }
        }
        return *(NearbyRegionBlock *) nullptr;
    }

    Biome & Map::getBiome(dword x, dword y, dword z) {
        dword region_block_x, region_block_y;
        if (getRegionBlockCoord(x, y, z, region_block_x, region_block_y)) {
            RegionBlock * blockx = world->region_blocks[region_block_x];
            if (blockx) {
                return *world->biomes[blockx[region_block_y].biome_index];
            }
        }
        return *(Biome *) nullptr;
    }

    Biome::Layer & Map::getLayer(dword x, dword y, dword z) {
        Block3d & block = getBlock3d(x, y, z);
        Biome & biome = getBiome(x, y, z);
        if (&block && &biome) {
            dword local_x = x & 15, local_y = y & 15;
            Biome::Layer & layer = *biome.layers[block.tile_info[local_x][local_y].layer_index];
            return layer;
        } else {
            return *(Biome::Layer *) nullptr;
        }
    }

    int32 Map::getLayerSpecies(dword x, dword y, dword z) {
        Biome::Layer & layer = getLayer(x, y, z);
        if (&layer) {
            return layer.layer_inorganic;
        } else {
            return -1;
        }
    }

    int32 Map::getMineralSpecies(dword x, dword y, dword z) {
        Block3d & block = getBlock3d(x, y, z);
        if (&block) {
            dword local_x = x & 15, local_y = y & 15;
            for (auto curr = block.tile_features.rbegin(), end = block.tile_features.rend(); curr < end; ++curr) {
                Mineral & mineral = *(Mineral *) *curr;
                if (&mineral &&
                    mineral.vtable->type(&mineral) == TileFeature::mineral &&
                    ((mineral.exists[local_y] >> local_x) & 1)) {
                    return mineral.species;
                }
            }
        }
        return -1;
    }

    int32 Map::getLavaStoneSpecies(dword x, dword y, dword z) {
        NearbyRegionBlock & nrb = getNearbyRegionBlock(x, y, z);
        if (&nrb) {
            return nrb.lava_stone_species;
        } else {
            return -1;
        }
    }

    void Map::getGrass(dword x, dword y, dword z, byte & density, int32 & species) {
        Block3d & block = getBlock3d(x, y, z);
        if (&block) {
            dword local_x = x & 15, local_y = y & 15;
            for (auto curr = block.tile_features.rbegin(), end = block.tile_features.rend(); curr < end; ++curr) {
                Grass & grass = *(Grass *) *curr;
                if (&grass &&
                    grass.vtable->type(&grass) == TileFeature::grass &&
                    (density = grass.density[local_x][local_y])) {
                    species = grass.species;
                    return;
                }
            }
        }
        density = 0;
        species = -1;
    }

    void Map::getContaminant(dword x, dword y, dword z, byte & size, int16 & material, int32 & species) {
        Block3d & block = getBlock3d(x, y, z);
        if (&block) {
            dword local_x = x & 15, local_y = y & 15;
            for (auto curr = block.tile_features.rbegin(), end = block.tile_features.rend(); curr < end; ++curr) {
                Contaminant & contaminant = *(Contaminant *) *curr;
                if (&contaminant &&
                    contaminant.vtable->type(&contaminant) == TileFeature::contaminant &&
                    (size = contaminant.size[local_x][local_y])) {
                    material = contaminant.material;
                    species = contaminant.species;
                    return;
                }
            }
        }
        size = 0;
        material = 0;
        species = -1;
    }

    UI::Base & Game::getTopmostUI() {
        UI::Base * curr = ui_bottom;
        if (curr) {
            while (curr->next) {
                curr = curr->next;
            }
        }
        return *curr;
    }

    dword Game::getFortressModeViewWidth() {
        int p1 = side_menu_pos, p2 = side_map_pos;
        switch (min(p1, p2)) {
        case 1:
            return view_width - 57;
        case 2:
            return view_width - 33;
        default:
            return view_width - 2;
        }
    }

    dword Game::getFortressModeViewHeight() {
        return view_height - 2;
    }

    string const & Game::seasonName() {
        static string const names[] = {
            "spring",
            "summer",
            "autumn",
            "winter"
        };
        return names[min((int) season, 3)];
    }

    dword Game::month() { // 1 ~ 12
        if (season >= 4 || season_time >= season_time_per_month * 3) { // erroneous data
            return 12;
        } if (season == 3 && season_time >= season_time_per_month) { // Jan & Feb
            return season_time / season_time_per_month;
        } else { // Mar ~ Dec
            return season * 3 + season_time / season_time_per_month + 3;
        }
    }

    string const & Game::dwarvenMonthName() {
        static string const names[] = {
            "granite",  // Mar
            "slate",    // Apr
            "felsite",  // Mar
            "hematite", // Jun
            "malachite",// Jul
            "galena",   // Aug
            "limestone",// Sep
            "sandstone",// Oct
            "timber",   // Nov
            "moonstone",// Dec
            "opal",     // Jan
            "obsidian", // Feb
        };
        return names[month() - 1];
    }

    dword Game::day() { // 1 ~ 28
        return season_time % season_time_per_month / season_time_per_day + 1;
    }

    string Game::dayName() {
        char v[5] = {0, 0, 0, 0, 0};
        dword day = this->day();
        dword ind;
        if (day < 10) {
            v[0] = '0' + day;
            ind = 1;
        } else {
            v[0] = '0' + day % 10;
            v[1] = '0' + day / 10;
            ind = 2;
        }
        if ((day >= 4 && day <= 20) || day >= 24) {
            v[ind] = 't';
            v[ind + 1] = 'h';
        } else if (day == 1 || day == 21) {
            v[ind] = 's';
            v[ind + 1] = 't';
        } else if (day == 2 || day == 22) {
            v[ind] = 'n';
            v[ind + 1] = 'd';
        } else if (day == 3 || day == 23) {
            v[ind] = 'r';
            v[ind + 1] = 'd';
        }
        return v;
    }

    static float const hardcode_mat_color[19][3] = {
        .5 , .5 , .5 ,  // rock
        1  , .75, 0  ,  // amber
        1  , 1  , 1  ,  // coral
        .01, .20, .13,  // glass_green
        .0 , .5 , .5 ,  // glass_clear
        1  , 1  , 1  ,  // glass_crystal
        1  , 1  , 1  ,  // water
        0  , 0  , 0  ,  // coal
        1  , 1  , 1  ,  // potash
        .5 , .5 , .5 ,  // ash
        1  , 1  , 1  ,  // pearlash
        .5 , .5 , .5 ,  // lye
        .59, .29, 0  ,  // mud
        .01, .20, .13,  // vomit
        1  , 1  , 1  ,  // salt
        .59, .29, 0  ,  // filth_b
        1  , 1  , 0  ,  // filth_y
        1  , 1  , 1  ,  // unknown_substance
        .5 , .5 , .5 ,  // grime
    };
//
//    static byte const hardcode_mat_console_color[19] = {
//        0x7,
//        0x6,
//        0xF,
//        0x2,
//        0x3,
//        0xF,
//        0xF,
//        0x0,
//        0xF,
//        0x7,
//        0xF,
//        0x7,
//        0x6,
//        0x2,
//        0xF,
//        0x6,
//        0xE,
//        0xF,
//        0x7,
//    };

    inline byte makeConsoleColor(byte console_color, byte console_color_bright) {
        if (((console_color >> 3) == 0) ^ (console_color_bright == 0)) { // this is the in-game behavior. bright value alters, rather than sets the brightness
            return (console_color & 7) | 8;
        } else {
            return (console_color & 7);
        }
    }

    float const * Game::getConsoleColor(byte console_color, byte console_color_bright) {
        return console_color_rgb[makeConsoleColor(console_color, console_color_bright)];
    }

    float const * Game::getHardCodedMaterialColor(int32 material) {
        if (material >= 0 && material < 19) {
            return hardcode_mat_color[material];
        } else {
            return hardcode_mat_color[HardCodeMaterial::unknown_substance];
        }
    }

    float const * Game::checkFineColor(int32 index, byte console_color, byte console_color_bright) {
        console_color = makeConsoleColor(console_color, console_color_bright);
        ColorDescriptor & desc = *map.descriptor_color[index];
        if (&desc) {
            byte desc_console_color = makeConsoleColor(desc.console_color, desc.console_color_bright);
            if (desc_console_color == console_color) {
                return (float const *) &desc.rgb;
            }
        }
        return console_color_rgb[console_color];
    }

    MaterialRaw & Game::getMaterial(int16 material, int32 species) {
        if (material == HardCodeMaterial::rock) {
            InorganicRaw & ino = *map.inorganic_raw[species];
            if (&ino) {
                return ino.material;
            }
        } else if (material < HardCodeMaterial::end) {
            return *(MaterialRaw *) 0; // todo
        } else if (material < HardCodeMaterial::end + 200) {
            return *(MaterialRaw *) 0; // todo
        } else if (material < HardCodeMaterial::end + 400) {
            return *(MaterialRaw *) 0; // todo
        } else {
            VegetationRaw & veg = *map.vegetation_raw[species];
            if (&veg) {
                return *veg.materials[material - HardCodeMaterial::end - 400];
            }
        }
        return *(MaterialRaw *) 0;
    }

    bool Game::isMetal(int16 material, int32 species) {
        if (material == HardCodeMaterial::rock) {
            InorganicRaw & ino = *map.inorganic_raw[species];
            if (&ino) {
                return ino.material.flags->is_metal;
            }
        } else if (material < HardCodeMaterial::end) {
            return false;
        } else if (material < HardCodeMaterial::end + 200) {
            return false; // todo
        } else if (material < HardCodeMaterial::end + 400) {
            return false; // todo
        } else {
            VegetationRaw & veg = *map.vegetation_raw[species];
            if (&veg) {
                MaterialRaw & mat = *veg.materials[material - HardCodeMaterial::end - 400];
                if (&mat) {
                    return mat.flags->is_metal;
                }
            }
        }
        return false;
    }

    bool Game::isGlass(int16 material, int32 species) {
        if (material == HardCodeMaterial::rock) {
            InorganicRaw & ino = *map.inorganic_raw[species];
            if (&ino) {
                return ino.material.flags->is_glass;
            }
        } else if (material == HardCodeMaterial::glass_green ||
                   material == HardCodeMaterial::glass_clear ||
                   material == HardCodeMaterial::glass_crystal) {
            return true;
        } else if (material < HardCodeMaterial::end) {
            return false;
        } else if (material < HardCodeMaterial::end + 200) {
            return false; // todo
        } else if (material < HardCodeMaterial::end + 400) {
            return false; // todo
        } else {
            VegetationRaw & veg = *map.vegetation_raw[species];
            if (&veg) {
                MaterialRaw & mat = *veg.materials[material - HardCodeMaterial::end - 400];
                if (&mat) {
                    return mat.flags->is_glass;
                }
            }
        }
        return false;
    }

    bool Game::isMetalOrGlass(int16 material, int32 species) {
        if (material == HardCodeMaterial::rock) {
            InorganicRaw & ino = *map.inorganic_raw[species];
            if (&ino) {
                return ino.material.flags->is_metal || ino.material.flags->is_glass;
            }
        } else if (material == HardCodeMaterial::glass_green ||
                   material == HardCodeMaterial::glass_clear ||
                   material == HardCodeMaterial::glass_crystal) {
            return true;
        } else if (material < HardCodeMaterial::end) {
            return false;
        } else if (material < HardCodeMaterial::end + 200) {
            return false; // todo
        } else if (material < HardCodeMaterial::end + 400) {
            return false; // todo
        } else {
            VegetationRaw & veg = *map.vegetation_raw[species];
            if (&veg) {
                MaterialRaw & templ = *veg.materials[material - HardCodeMaterial::end - 400];
                if (&templ) {
                    return templ.flags->is_metal || templ.flags->is_glass;
                }
            }
        }
        return false;
    }

    float const * Game::getColorOfMaterial(int16 material, int32 species, MaterialState::Enum state, TemplateColorType::Enum type, bool console_strict) {
        MaterialRaw * mat = 0;
        if (material == HardCodeMaterial::rock) {
            InorganicRaw & ino = *map.inorganic_raw[species];
            if (&ino) {
                mat = &ino.material;
            }
            if (!mat) {
                return hardcode_mat_color[HardCodeMaterial::rock];
            }
        } else if (material < HardCodeMaterial::end) {
            return getHardCodedMaterialColor(material);
        } else if (material < HardCodeMaterial::end + 200) {
            return hardcode_mat_color[HardCodeMaterial::unknown_substance]; // todo creature_raw
        } else if (material < HardCodeMaterial::end + 400) {
            return hardcode_mat_color[HardCodeMaterial::unknown_substance]; // todo generated?_creature_raw
        } else {
            VegetationRaw & veg = *map.vegetation_raw[species];
            if (&veg) {
                mat = veg.materials[material - HardCodeMaterial::end - 400];
            }
            if (!mat) {
                return hardcode_mat_color[HardCodeMaterial::unknown_substance];
            }
        }
        byte console_color = 0, console_color_bright = 0;
        if (mat) {
            switch (type) {
            case TemplateColorType::basic:
                console_color = mat->basic_color_fore;
                console_color_bright = mat->basic_color_fore_bright;
                break;
            case TemplateColorType::build:
//                console_color = templ->build_color_fore;
//                console_color_bright = templ->build_color_fore_bright;
                console_color = mat->build_color_back;
                console_color_bright = 0;
                break;
            case TemplateColorType::tile:
                console_color = mat->tile_color_fore;
                console_color = mat->tile_color_fore_bright;
                break;
            }
        }
        if (console_strict) {
            if (mat) {
                return checkFineColor(((int32 *) &mat->state_color)[state], console_color, console_color_bright); // check if the fine color matches the console color
            } else {
                return hardcode_mat_color[HardCodeMaterial::unknown_substance];
            }
        } else {
            if (mat) {
                ColorDescriptor & desc = *map.descriptor_color[((int32 *) &mat->state_color)[state]];
                if (&desc) {
                    return (float const *) &desc.rgb;
                }
                return getConsoleColor(console_color, console_color_bright);
            } else {
                return hardcode_mat_color[HardCodeMaterial::unknown_substance];
            }
        }
    }

    float const * Game::getColorOfMaterial(int16 material, int32 species, MaterialState::Enum state, byte console_color, byte console_color_bright, bool console_strict) {
        MaterialRaw * mat = 0;
        if (material == HardCodeMaterial::rock) {
            InorganicRaw & ino = *map.inorganic_raw[species];
            if (&ino) {
                mat = &ino.material;
            }
        } else if (material < HardCodeMaterial::end) {
        } else if (material < HardCodeMaterial::end + 200) { // todo creature_raw
        } else if (material < HardCodeMaterial::end + 400) { // todo generater?_creature_raw
        } else {
            VegetationRaw & veg = *map.vegetation_raw[species];
            if (&veg) {
                mat = veg.materials[material - HardCodeMaterial::end - 400];
            }
        }
        if (console_strict) {
            if (mat) {
                return checkFineColor(((int32 *) &mat->state_color)[state], console_color, console_color_bright);
            } else {
                return getConsoleColor(console_color, console_color_bright);
            }
        } else {
            if (mat) {
                ColorDescriptor & desc = *map.descriptor_color[((int32 *) &mat->state_color)[state]];
                if (&desc) {
                    return (float const *) &desc.rgb;
                }
            }
            return getConsoleColor(console_color, console_color_bright);
        }
    }
}

#endif

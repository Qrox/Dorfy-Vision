#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_TEMPLATE_H__
#define __DORFY_VISION__DWARF_TEMPLATE_H__

#include "DwarfDataGeneral.h"

namespace DwarfFortress {
    namespace HardCodeMaterial {
        enum Enum {
            rock,
            amber,
            coral,
            glass_green,
            glass_clear,
            glass_crystal,
            water,
            coal,
            potash,
            ash,
            pearlash,
            lye,
            mud,
            vomit,
            salt,
            filth_b,
            filth_y,
            unknown_substance,
            grime,
            end
        };
    }

    namespace MaterialState {
        enum Enum {
            solid,
            liquid,
            gas,
            soild_powder,
            solid_paste,
            solid_pressed,
        };
    }

    namespace TemplateColorType {
        enum Enum {
            basic,
            build,
            tile,
        };
    }

    union MaterialRaw {
        struct Mechanics {
            dword
            bending,
            shear,
            torsion,
            impact,
            tensile,
            compressive;
        };

        __at(0, String identifier);
        __at(1C, String gem_name_singular);
        __at(38, String gem_name_plural);
        __at(54, String stone_name);
        __at(70, word specific_heat);
        __at(72, word heatdam_point);
        __at(74, word colddam_point);
        __at(76, word ignition_point);
        __at(78, word melting_point);
        __at(7A, word boiling_point);
        __at(7C, word fixed_temp);
        __at(80, dword solid_density);
        __at(84, dword liquid_density);
        __at(88, dword molar_mass);
        __at(8C, struct {
            int32
            solid,
            liquid,
            gas,
            solid_powder,
            solid_paste,
            solid_pressed;
        } state_color);
        __at(A4, struct {
            String
            solid,
            liquid,
            gas,
            solid_powder,
            solid_paste,
            solid_pressed;
        } state_name);
        __at(14C, struct {
            String
            solid,
            liquid,
            gas,
            solid_powder,
            solid_paste,
            solid_pressed;
        } state_adj);
        __at(1F4, dword absorption);
        __at(1F8, Mechanics yield);
        __at(210, Mechanics fracture);
        __at(228, Mechanics strain_at_yield);
        __at(240, dword max_edge);
        __at(244, dword value);
        __at(248, struct {
            dword
            bone : 1,                   //0
            meat : 1,                   //1
            edible_vermin : 1,          //2
            edible_raw : 1,             //3
            edible_cooked : 1,          //4
            alcohol : 1,                //5
            items_metal : 1,            //6
            items_barred : 1,           //7
            items_scaled : 1,           //8
            items_leather : 1,          //9
            items_soft : 1,             //10
            items_hard : 1,             //11
            implies_animal_kill : 1,    //12
            alcohol_plant : 1,          //13
            alcohol_creature : 1,       //14
            cheese_plant : 1,           //15
            cheese_creature : 1,        //16
            powder_misc_plant : 1,      //17
            powder_misc_creature : 1,   //18
            stockpile_glob : 1,         //19
            liquid_misc_plant : 1,      //20
            liquid_misc_creature : 1,   //21
            liquid_misc_other : 1,      //22
            wood : 1,                   //23
            thread_plant : 1,           //24
            tooth : 1,                  //25
            horn : 1,                   //26
            pearl : 1,                  //27
            shell : 1,                  //28
            leather : 1,                //29
            silk : 1,                   //30
            soap : 1,                   //31
            rots : 1,                   //32
            unknown_33 : 1,             //33
            unknown_34 : 1,             //34
            liquid_misc : 1,            //35
            structural_plant_mat : 1,   //36
            seed_mat : 1,               //37
            unknown_38 : 1,             //38
            cheese : 1,                 //39
            enters_blood : 1,           //40
            blood_map_descriptor : 1,   //41
            ichor_map_descriptor : 1,   //42
            goo_map_descriptor : 1,     //43
            slime_map_descriptor : 1,   //44
            pus_map_descriptor : 1,     //45
            generates_miasma : 1,       //46
            is_metal : 1,               //47
            unknown_48 : 1,             //48
            is_glass : 1,               //49
            crystal_glassable : 1,      //50
            items_weapon : 1,           //51
            items_weapon_ranged : 1,    //52
            items_anvil : 1,            //53
            items_ammo : 1,             //54
            items_digger : 1,           //55
            items_armor : 1,            //56
            items_delicate : 1,         //57
            items_siege_engine : 1,     //58
            items_quern : 1,            //59
            is_stone : 1,               //60
            undiggable : 1,             //61
            yarn : 1,                   //62
            stockpile_glob_paste : 1,   //63
            stockpile_glob_pressed : 1, //64
            display_unglazed : 1,       //65
            do_not_clean_glob : 1,      //66
            no_stone_stockpile : 1,     //67
            stockpile_thread_metal : 1, //68
            sweat_map_descriptor : 1,   //69
            tear_map_descriptor : 1,    //70
            spit_map_descriptor : 1,    //71
            evaporates : 1;             //72
        } * flags);
        __at(252, word butcher_item_type);
        __at(254, word butcher_item_subtype);
        __at(258, String meat_name);
        __at(274, String meat_adj);
        __at(290, String meat_prefix);
        __at(2AC, String block_name_singular);
        __at(2C8, String block_name_plural);
        __at(38C, String hardens_with_water_material_string);
        __at(3A8, String hardens_with_water_species_string);
        __at(3E0, Set<String *> reaction_class);
        __at(3F0, byte tile);
        __at(3F2, word basic_color_fore);
        __at(3F4, word basic_color_fore_bright);
        __at(3F6, word build_color_fore);
        __at(3F8, word build_color_back);
        __at(3FA, word build_color_fore_bright);
        __at(3FC, word tile_color_fore);
        __at(3FE, word tile_color_back);
        __at(400, word tile_color_fore_bright);
        __at(402, byte item_symbol);
        __at(418, dword soap_level);
        __at(42C, String prefix);
    };
}

#endif

#endif

#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_ITEM_H__
#define __DORFY_VISION__DWARF_ITEM_H__

#include "DwarfDataGeneral.h"

namespace DwarfFortress {
    namespace ItemType {
        enum Enum : dword {
            bar,
            gem_small,
            block,
            gem_rough,
            stone,
            wood,
            door,
            flood_gate,
            bed,
            chair,
            chain,
            flask,
            goblet,
            instrument,
            toy,
            window,
            cage,
            barrel,
            bucket,
            animal_trap,
            table,
            coffin,
            statue,
            corpse,
            weapon,
            armor,
            shoes,
            shield,
            helm,
            gloves,
            box,
            bin,
            armor_stand,
            weapon_rack,
            cabinet,
            figurine,
            amulet,
            sceptor,
            ammo,
            crown,
            ring,
            earring,
            bracelet,
            gem_large,
            anvil,
            body_part,
            remain,
            meat,
            fish_prepared,
            fish_raw,
            vermin_untamed,
            vermin_tamed,
            seed,
            plant,
            skin_tanned,
            plant_growth,
            thread,
            cloth,
            totem,
            pants,
            backpack,
            quiver,
            catapult_part,
            ballista_part,
            siege_ammo,
            ballista_arrow_head,
            trap_part,
            trap_component,
            drink,
            powder,
            cheese,
            food,
            liquid,
            coin,
            glob,
            rock_small,
            pipe_section,
            hatch_cover,
            grate,
            quern,
            millstone,
            splint,
            crutch,
            traction_bench,
            cast,
            tool,
            slab,
            egg,
            book,
        };
    }

    namespace GemCut {
        enum Enum : dword {
            gizzard_stones = 11,
            smooth_pebbles,
            oval_cabochon,
            round_cabochon,
            cushion_cabochon,
            rectangular_cabochon,
            point_cut,
            table_cut,
            single_cut,
            rose_cut,
            briolette_cut,
            emerald_cut,
            marquise_cut,
            oval_cut,
            pear_cut,
            square_brilliant_cut,
            radiant_cut,
            trillion_cut,
            round_brilliant_cut,
            baguette_cut,
            tapered_baguette_cut,
            cushion_cut,
            octagon_cut,
            square_cut,
        };
    }

    union Item {
        struct Flags {
            dword
            unknown_0 : 1,
            unknown_1_7 : 7,
            rotten : 1,
            unknown_9_13 : 5,
            imported : 1,
            unknown_15_18 : 4,
            forbidden : 1,
            unknown_20 : 1,
            dump : 1,
            on_fire : 1,
            unknown_23 : 1,
            hidden : 1,
            unknown_25_27 : 3,
            temperature_calculated : 1,
            weight_calculated : 1,
            unknown_30_31 : 2;
        };

        __at(0, union {
            __at(0, ItemType::Enum (__thiscall * getItemType)(Item *));
            __at(14, void (__thiscall * setMaterial)(Item *, int16 material));
            __at(18, void (__thiscall * setSpecies)(Item *, int32 species));
            __at(1C, int16 (__thiscall * getMaterial)(Item *));
            __at(20, int32 (__thiscall * getSpecies)(Item *));
            __at(94, word (__thiscall * getSpecificHeat)(Item *));
            __at(98, word (__thiscall * getIgnitionPoint)(Item *));
            __at(9C, word (__thiscall * getHeatDamPoint)(Item *));
            __at(A0, word (__thiscall * getColdDamPoint)(Item *));
            __at(A4, word (__thiscall * getBoilingPoint)(Item *));
            __at(A8, word (__thiscall * getMeltingPoint)(Item *));
            __at(AC, word (__thiscall * getFixedTemperature)(Item *));
            __at(B8, word (__thiscall * getTemperature)(Item *)); // not sure
            __at(E8, word (__thiscall * getWear)(Item *));
            __at(EC, void (__thiscall * setWear)(Item *, word wear));
            __at(23C, dword (__thiscall * getCount)(Item *));
            __at(240, void (__thiscall * increaseCount)(Item *, int32 inc));
            __at(244, void (__thiscall * setCount)(Item *, int32 cnt));
            __at(284, word (__thiscall * getQuality)(Item *));
            __at(288, word (__thiscall * getHighestQualityOnItem)(Item *)); // of the item and its improvements
            __at(28C, word (__thiscall * getHighestQualityOfImprovements)(Item *));
            __at(2E4, byte (__thiscall * hasImprovements)(Item *));
            __at(2EC, dword (__thiscall * isMagical)(Item *));
            __at(39C, GemCut::Enum (__thiscall * getGemCut)(Item *));
        } * vtable);
        __at(4, Vector3<int16> location);
        __at(C, Flags flags);
        __at(18, dword index);
        __at(5C, dword weight);
        __at(64, dword count);
        __at(6C, dword magical);
        __at(78, word wear);
        __at(88, int16 material);
        __at(8A, int16 bar_material);
        __at(8C, int32 species);
        __at(90, word gem_cut);
        __at(90, word metal_dimesion);
        __at(92, word quality);
    };
}

#endif

#endif

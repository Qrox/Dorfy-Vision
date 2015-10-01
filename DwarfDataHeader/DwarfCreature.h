#ifdef BUILD_DLL

#ifndef __DORFY_VISION__DWARF_CREATURE_H__
#define __DORFY_VISION__DWARF_CREATURE_H__

#include "DwarfDataGeneral.h"

#undef small // who the fuck defined this macro!

namespace DwarfFortress {
    namespace TimeScale {
        enum Enum {
            daily = 0,
            yearly = 3
        };
    }

    namespace GeneticModel {
        enum Enum {
            mix = 0,
            dominant_less = 1,
            dominant_more = 2
        };
    }

    namespace Habitat {
        enum Enum {
            terrestrial = 0,
            aquatic = 1,
            amphibious = 3
        };
    }

    namespace SoundType {
        enum Enum {
            alert = 0,
            peaceful_intermittent = 1
        };
    }

    namespace SoundRequirement {
        enum Enum {
            none = -1,
            vocalization = 0
        };
    }

    namespace TissueShape {
        enum Enum : word {
            layer,
            strands,
            feather,
            scales
        };
    }

    namespace SpecialAttackType {
        enum Enum {
            inject_extract,
            suck_blood,
            interation
        };
    }

    namespace CreatureInteractionType {
        enum Enum {
            retract_into_bodypart,
            interaction,
            root_around,
        };
    }

    namespace BreathType {
        enum Enum {
            trailing_dust_flow,
            trailing_vapor_flow,
            trailing_gas_flow,
            solid_glob,
            liquid_glob,
            undirected_gas,
            undirected_vapor,
            undirected_dust,
            web_spray,
            dragonfire,
            firejet,
            fireball,
            weather_creeping_gas,
            weather_creeping_vapor,
            weather_creeping_dust,
            weather_falling_material,
            spatter_powder,
            spatter_liquid,
            undirected_item_cloud,
            trailing_item_flow,
        };
    }

    namespace UsageHint {
        enum Enum {
            major_curse,
            greeting,
            clean_self,
            clean_friend,
            attack,
            fleeing,
            negative_social_responce,
            torment,
        };
    }

    namespace LocationHint {
        enum Enum {
            in_water,
            in_magma,
            no_water,
            no_magma,
        };
    }

    namespace SecretionTrigger {
        enum Enum {
            continuous,
            extreme_emotion,
            exertion,
        };
    }

    namespace Profession {
        enum Enum {
            miner,
            woodworker,
            carpenter,
            bowyer,
            woodcutter,
            stoneworker,
            engraver,
            mason,
            ranger,
            animal_caretaker,
            animal_trainer,
            hunter,
            trapper,
            animal_dissector,
            metalsmith,
            furnace_operator,
            weaponsmith,
            armorer,
            blacksmith,
            metalcrafter,
            jeweler,
            gem_cutter,
            gem_setter,
            craftsdwarf,
            woodcrafter,
            stonecrafter,
            leatherworker,
            bone_carver,
            weaver,
            clothier,
            glassmaker,
            potter,
            glazer,
            wax_worker,
            strand_extractor,
            fishery_worker,
            fisherdwarf,
            fish_dissector,
            fish_cleaner,
            farmer,
            cheese_maker,
            milker,
            cook,
            tresher,
            miller,
            butcher,
            tanner,
            dyer,
            planter,
            herbalist,
            brewer,
            soap_maker,
            potash_maker,
            lye_maker,
            wood_burner,
            shearer,
            spinner,
            presser,
            beekeeper,
            engineer,
            mechanic,
            siege_engineer,
            siege_operator,
            pump_operator,
            clerk,
            administrator,
            trader,
            architect,
            alchemist,
            doctor,
            diagnoser,
            bone_doctor,
            suturer,
            surgeon,
            merchant,
            hammerdwarf,
            hammer_lord,
            speardwarf,
            spearmaster,
            marksdwarf,
            elite_marksdwarf,
            wrestler,
            elite_wrestler,
            axedwarf,
            axe_lord,
            swordsdwarf,
            swordmaster,
            macedwarf,
            mace_lord,
            pikedwarf,
            pikemaster,
            bowdwarf,
            elite_bowdwarf,
            blowgunner,
            master_blowgunner,
            lasher,
            master_lasher,
            recruit,
            hunting_animal,
            war_animal,
            master_thief,
            thief,
            peasant,
            dwarven_child,
            dwarven_baby,
            drunk,
            monster_slayer,
            scout,
            beast_hunter,
            snatcher,
            mercenary,
            PROFESSION_COUNT
        };
    }

    union CreatureRaw {
        union Gait {
            __at(4, dword full_speed);
            __at(8, dword buildup_time);
            __at(C, dword max_turning_speed);
            __at(10, dword start_speed);
            __at(14, dword energy_use);
            __at(1C, dword stealth_slow_percentage);
        };

        union LayerRaw {
            __at(0, String identifier);
            __at(1C, dword tissue_id);
            __at(28, dword layer_thickness);
            __at(2C, dword coverage); //?
            __at(3A, int32 relation_type);
            __at(3C, int32 relation_bodypart);
            __at(50, dword creature_layer_id);
            __at(54, int32 attached_bodypart_layer_id);
            __at(58, int32 attached_creature_layer_id);
            __at(5C, int32 outer_bodypart_layer_id);
        };

        union BodyPartRaw {
            struct Flags {
                dword
                head : 1,
                upperbody : 1,
                lowerbody : 1,
                sight : 1,
                embedded : 1,
                internal : 1,
                circulation : 1,
                skeleton : 1,

                limb : 1,
                grasp : 1,
                stance : 1,
                guts : 1,
                breathe : 1,
                small : 1,
                throat : 1,
                joint : 1,

                thought : 1,
                nervous : 1,
                right : 1,
                left : 1,
                hear : 1,
                smell : 1,
                flier : 1,
                digit : 1,

                mouth : 1,
                aperture : 1,
                socket : 1,
                totemable : 1,
                unknown_28 : 1,
                unknown_29 : 1,
                under_pressure : 1,
                unknown_31 : 1, // all body parts

                vermin_butcher_item : 1,
                connector : 1,
                unknown_34 : 1,
                unknown_35 : 1,
                prevents_parent_collapse : 1;
            };

            __at(0, String identifier);
            __at(1C, String category);
            __at(38, word connection);
            __at(3C, Flags * flags);
            __at(44, Set<LayerRaw *> layers);
            __at(64, dword relative_size);
            __at(68, dword count);
            __at(70, Set<String *> names_singular);
            __at(80, Set<String *> names_plural);
        };

        union TissueRaw {
            struct Flags {
                dword
                thickens_on_strength : 1,
                thickens_on_energy_storage : 1,
                arteries : 1,
                scars : 1,
                structural : 1,
                nervous : 1,
                thought : 1,
                muscular : 1,

                smell : 1,
                hear : 1,
                flight : 1,
                breathe : 1,
                sight : 1,
                cosmetic : 1,
                connects : 1,
                functional : 1,

                major_arteries : 1,
                tissue_leaks : 1,
                styleable : 1,
                connective_tissue_anchor : 1,
                settable : 1,
                splintable : 1;
            };

            __at(0, String identifier);
            __at(1C, Flags * flags);
            __at(24, String name_singular);
            __at(40, String name_plural);
            __at(B0, int16 tissue_material);
            __at(B4, int32 tissue_species);
            __at(B8, dword relative_thickness);
            __at(BC, dword healing_rate);
            __at(C0, dword vascular);
            __at(C4, dword pain_receptors);
            __at(C8, word tissue_shape);
            __at(D0, word insulation);
            __at(F0, int32 subordinate_to_tissue_id);
            __at(F4, word tissue_material_state);
        };

        union CasteRaw {
            struct Flags {
                dword
                habitat : 2,
                lockpicker : 1,
                mischievious : 1,
                pattern_flier : 1,
                curious_beast : 1,
                curious_beast_item : 1,
                curious_beast_guzzler : 1,

                flee_quick : 1,
                at_peace_with_wildlife : 1,
                can_swim : 1,
                opposed_to_life : 1,
                curious_beast_eater : 1,
                no_eat : 1,
                no_drink : 1,
                no_sleep : 1,

                common_domestic : 1,
                wagon_puller : 1,
                pack_animal : 1,
                flier : 1,
                large_predator : 1,
                magma_vision : 1,
                fire_immune : 1,
                dragonfire_immune : 1,

                unknown_24 : 1,
                unknown_25 : 1,
                fish_item : 1,
                immobile_land : 1,
                immolate : 1,
                unknown_29 : 1,
                no_spring : 1,
                no_summer : 1,

                no_autumn : 1,
                no_winter : 1,
                benign : 1,
                vermin_noroam : 1,
                vermin_notrap : 1,
                vermin_nofish : 1,
                has_nerves : 1,
                no_dizziness : 1,

                no_fevers : 1,
                no_unit_type_color : 1,
                no_connections_for_movement : 1,
                supernatural : 1,
                ambush_predator : 1,
                not_butcherable : 1,
                unknown_46 : 1, // some FBs, demons, some deities
                cookable_live : 1,

                unknown_48 : 1,
                immobile : 1,
                multipart_full_vision : 1,
                meanderer : 1,
                thick_web : 1,
                trainable_hunting : 1,
                pet : 1,
                pet_exotic : 1,

                unknown_56 : 1, // some FBs, the titan, NCs, BMs, WBs, some deities
                can_speak : 1,
                can_learn : 1,
                utterances : 1,
                bone_carnivore : 1,
                carnivore : 1,
                underswim : 1,
                no_exert : 1,

                no_pain : 1,
                extravision : 1,
                no_breathe : 1,
                no_stun : 1,
                no_nausea : 1,
                unknown_69 : 1, // some FBs, the titan, NCs, BMs, WBs, some deities
                trances : 1,
                no_emotion : 1,

                slow_learner : 1,
                no_stuckins : 1,
                unknown_74 : 1,
                no_skull : 1,
                no_skin : 1,
                no_bones : 1,
                no_meat : 1,
                no_paralyze : 1, //PARALYZE_IMMUNE

                no_fear : 1,
                can_open_doors : 1,
                likes_fighting : 1,
                gets_wound_infections : 1,
                no_miasma : 1, // NO_SMELLYROT
                remains_undetermined : 1,
                has_shell : 1,
                pearl : 1,

                trainable_war : 1,
                no_thought_center_for_movement : 1,
                arena_restricted : 1,
                lair_hunter : 1,
                unknown_92 : 1,
                vermin_hateable : 1,
                vegetation : 1,
                magical : 1,

                natural : 1,
                unknown_97 : 1,
                unknown_98 : 1,
                multiple_litter_rare : 1,
                mount : 1,
                mount_exotic : 1,
                feature_attack_group : 1,
                vermin_micro : 1,

                equips : 1,
                lays_eggs : 1,
                standard_grazer : 1,
                no_thought : 1,
                trap_avoid : 1,
                cave_adapt : 1,
                megabeast : 1,
                semimegabeast : 1,

                all_active : 1,
                diurnal : 1,
                nocturnal : 1,
                crepuscular : 1,
                matutinal : 1,
                vespertine : 1,
                light_gen : 1,
                lisp : 1,

                gets_infections_from_rot : 1,
                unknown_121 : 1,
                alcohol_dependent : 1,
                swims_innate : 1,
                power : 1,
                unknown_125 : 1, // some FBs, NCs, BMs, WBs, some deities
                unknown_126 : 1, // some FBs, NCs, BMs, WBs, some deities
                unknown_127 : 1,

                unknown_128 : 1,
                unknown_129 : 1,
                unknown_130 : 1,
                unknown_131 : 1, // forgotten_beasts
                unknown_132 : 1, // titans
                unknown_133 : 1,
                unknown_134 : 1, // demons
                mannerism_laugh : 1,

                mannerism_smile : 1,
                mannerism_walk : 1,
                mannerism_sit : 1,
                mannerism_breath : 1,
                mannerism_posture : 1,
                mannerism_stretch : 1,
                mannerism_eyelids : 1,
                night_creature : 1,

                night_creature_hunter : 1,
                night_creature_bogeyman : 1,
                converted_spouse : 1,
                spouse_converter : 1,
                spouse_conversion_target : 1,
                vermin_die_when_bite : 1,
                vermin_remiains_on_bite_death : 1,
                hover_around_colony : 1, // COLONY_EXTERNAL

                unknown_152 : 1,
                returns_vermin_kills_to_owner : 1,
                hunts_vermin : 1,
                adopts_owner : 1,
                unknown_156 : 1,
                unknown_157 : 1,
                not_living : 1,
                no_phys_att_gain : 1,

                no_phys_att_rust : 1,
                crazed : 1,
                blood_sucker : 1,
                no_vegetation_perturb : 1,
                dive_hunts_vermin : 1,
                unknown_165 : 1,
                cannot_jump : 1,
                stance_climber : 1,

                cannot_climb : 1;
            };

            struct AttributeRate {
                dword
                improve_cost,
                unused_counter,
                rust_counter,
                demotion_counter;
            };

            struct AppearanceModifier {
                word type, unknown_2;
                dword range[7], description_range[6], rate;
                word time_scale, unknown_3E;
                dword growth_min, growth_max, day_begin, day_end, importance;
                String name;
                word unknown_70, genetic_model;
            };

            struct MaterialForceMultiplier {
                String tags[3];
                int32 material, species;
                dword numerator, denominator;
            };

            struct CreatureSound {
                dword type, range, delay, requirement;
                String verb, verb_3rd, noun;
            };

            struct ExtraButcherItem {
                word bodypart_id, unknown_2;
                String gem_shape_tag;
                int32 gem_shape;
                String item_type_tag, item_subtype_tag, material_tag[3];
                int16 item_type, item_subtype, material, unknown_B6;
                int32 species;
                dword any_hard_stone;
            };

            struct Secretion {
                int16 material, unknown_2;
                int32 species;
                word state, unknown_A;
                String material_tags[3];
                Set<word> bodyparts;
                Set<word> tissue;
                dword trigger;
            };

            union CreatureInteraction {
                struct Flags {
                    dword
                    can_be_mutual : 1,
                    verbal : 1,
                    free_action : 1;
                };

                struct TargetType {
                    dword
                    line_of_sight : 1,
                    touchable : 1,
                    disturber_only : 1,
                    self_allowed : 1,
                    self_only : 1;
                };

                __at(0, dword type);
                __at(4, Set<String *> by_what);
                __at(14, Set<String *> what);
                __at(24, String item_type_tag);
                __at(40, String item_subtype_tag);
                __at(5C, String material_tags[3]);
                __at(B0, word breath_type);
                __at(B4, String verb);
                __at(D0, String verb_3rd);
                __at(EC, String verb_mutual);
                __at(108, String reverse_verb);
                __at(124, String reverse_verb_3rd);
                __at(140, String target_verb);
                __at(15C, String target_verb_3rd);
                __at(194, int32 interaction_id);
                __at(198, Set<dword> usage_hint);
                __at(1A8, Set<dword> location_hint);
                __at(1B8, Flags flags);
                __at(1BC, Set<String *> target_token);
                __at(1CC, Set<TargetType> target_type);
                __at(1DC, Set<dword> target_range);
                __at(1EC, Set<String *> max_target_token);
                __at(1FC, Set<dword> max_target_count); // corresponds to max_target_token, not target_token
                __at(22C, String adventure_mode_name);
                __at(248, dword wait_period);
            };

            union CreatureAttack {
                struct Flags {
                    dword
                    prompt_with_bodypart : 1,
                    can_latch : 1,
                    main_attack : 1,
                    edge : 1,
                    multiattack_no_penalty : 1,
                    multiattack_penalty : 1;
                };

                __at(0, String identifier);
                __at(1C, String verb_3rd);
                __at(38, String verb);
                __at(54, Flags flags);
                __at(58, Set<dword> special_attack_type);
                __at(68, Set<int16> special_attack_injection_material);
                __at(78, Set<int32> special_attack_injection_species);
                __at(88, Set<word> special_attack_injection_state);
                __at(C8, Set<dword> special_attack_amount_min);
                __at(D8, Set<dword> special_attack_amount_max);
                __at(E8, dword contact_percentage);
                __at(EC, dword penetration_percentage);
                __at(F0, word bodypart_id);
                __at(114, dword skill_id);
                __at(118, dword velocity_modifier); // divide this by 1000
                __at(12C, Set<dword> special_attack_interaction_id);
                __at(13C, dword prepare_ticks); // adventure mode ticks
                __at(140, dword recover_ticks); // adventure mode ticks
            };

            __at(0, String identifier);
            __at(1C, String name_singular);
            __at(38, String name_plural);
            __at(54, String adjective);
            __at(70, String vermin_bite_verb);
            __at(8C, String gnaw_verb);
            __at(A8, String baby_name_singular);
            __at(C4, String baby_name_plural);
            __at(E0, String child_name_singular);
            __at(FC, String child_name_plural);
            __at(1A4, String remains_name_singular);
            __at(1C0, String remains_name_plural);
            __at(1DC, String description);
            __at(1F8, String mannerism_finger);
            __at(214, String mannerism_fingers);
            __at(230, String mannerism_nose);
            __at(24C, String mannerism_ear);
            __at(268, String mannerism_head);
            __at(284, String mannerism_eye);
            __at(2A0, String mannerism_mouth);
            __at(2BC, String mannerism_hair);
            __at(2D8, String mannerism_knuckle);
            __at(2F4, String mannerism_lip);
            __at(310, String mannerism_cheek);
            __at(32C, String mannerism_nail);
            __at(348, String mannerism_foot);
            __at(364, String mannerism_arm);
            __at(380, String mannerism_hand);
            __at(39C, String mannerism_tongue);
            __at(3B8, String mannerism_leg);
            __at(3D4, byte tile);
            __at(3D5, byte soldier_tile);
            __at(3D6, byte tile_alt);
            __at(3D7, byte soldier_tile_alt);
            __at(3D8, byte glow_tile);
            __at(3DA, word homeotherm);
            __at(3E0, word fixed_temp);
            __at(3E2, word color_fore);
            __at(3E4, word color_back);
            __at(3E6, word color_fore_bright);
            __at(3E8, word litter_size_max);
            __at(3EA, word litter_size_min);
            __at(3EC, word container_penetrate_power);
            __at(3EE, word vermin_bite_probability);
            __at(3F0, word grass_trample);
            __at(3F2, word building_destroyer);
            __at(3F4, int16 corpse_item_type);
            __at(3F6, int16 corpse_item_subtype);
            __at(3F8, int16 corpse_item_material);
            __at(3FA, int16 corpse_item_species);
            __at(3FC, word corpse_item_quality);
            __at(3FE, word remains_color_fore);
            __at(400, word remains_color_back);
            __at(402, word remains_color_fore_bright);
            __at(404, word difficulty);
            __at(406, word glow_color_fore);
            __at(408, word glow_color_back);
            __at(40A, word glow_color_fore_bright);
            __at(40C, word beaching_frequency);
            __at(40E, word clutch_size_min);
            __at(410, word clutch_size_max);
            __at(412, word vision_arc_binocular);
            __at(414, word vision_arc_nonbinocular);
            __at(41C, dword mod_value);
            __at(420, dword pet_value);
            __at(424, dword milk_frequency);
            __at(428, dword view_range);
            __at(42C, dword max_age_min);
            __at(430, dword max_age_max);
            __at(434, dword babyhood_end);
            __at(438, dword childhood_end);
            __at(440, dword trade_capacity);
            __at(448, dword population_ratio);
            __at(44C, dword max_body_size); // max BODY_SIZE / 10
            __at(454, dword species_id);
            __at(46C, dword egg_size); // EGG_SIZE / 10
            __at(470, dword grazer);
            __at(474, dword pet_value_divisor);
            __at(478, dword prone_to_rage);
            __at(47C, dword general_material_force_multiplier_numerator);
            __at(480, dword general_material_force_multiplier_denominator);
            __at(4F0, word personality_min[50]);
            __at(554, word personality_middle[50]);
            __at(5B8, word personality_max[50]);
            __at(61C, Flags * flags);
            __at(624, dword global_caste_id);
            __at(628, Set<BodyPartRaw *> bodyparts);
            __at(638, Set<CreatureAttack *> attacks);
            __at(648, Set<CreatureInteraction *> interactions); // root_around, retract_into_bodypart, and standard interactions
            __at(658, Set<ExtraButcherItem *> extra_butcher_items);
            __at(6D0, Set<Gait *> gaits);
            __at(77C, Set<String *> speech_file);
            __at(78C, dword skill_rate_percentage[117]);
            __at(960, dword skill_rate_unused_counter[117]);
            __at(B34, dword skill_rate_rust_counter[117]);
            __at(D08, dword skill_rate_demotion_counter[117]);
            __at(EDC, dword attribute_range[19][7]);
            __at(10F0, AttributeRate attribute_rate[19]);
            __at(1220, dword attribute_cap_percentage[19]);
            __at(1270, dword orientation_male_disinterested);
            __at(1274, dword orientation_male_lover);
            __at(1278, dword orientation_male_commitment);
            __at(127C, dword orientation_female_disinterested);
            __at(1280, dword orientation_female_lover);
            __at(1284, dword orientation_female_commitment);
            __at(1288, Set<dword> body_size_stage);
            __at(1298, Set<dword> body_size_time); // in days
            __at(12A8, Set<AppearanceModifier *> body_appearance_modifier);
            __at(12B8, Set<AppearanceModifier *> bodypart_apperance_modifier);
            __at(12C8, Set<dword> bodypart_apperance_modifier_id);
            __at(12D8, Set<word> bodypart_appearance_bodypart_id);
            __at(13E0, Set<word> natural_skill_id);
            __at(1400, Set<dword> natural_skill_level);
            __at(1410, String profession_name_singular[111]);
            __at(2034, String profession_name_plural[111]);
            __at(2C58, Set<int16> extract_material);
            __at(2C68, Set<int32> extract_species);
            __at(2CA8, int16 milk_material);
            __at(2CAC, int32 milk_species);
            __at(2D04, int16 web_material);
            __at(2D08, int32 web_species);
            __at(2D60, int16 vermin_bite_injection_material);
            __at(2D64, int32 vermin_bite_injection_species);
            __at(2D68, word vermin_bite_injection_state);
            __at(2DC0, int16 tendon_material);
            __at(2DC4, int32 tendon_species);
            __at(2E1C, dword tendon_healing_rate);
            __at(2E20, int16 ligament_material);
            __at(2E24, int32 ligament_species);
            __at(2E7C, dword ligament_healing_rate);
            __at(2E80, word blood_state);
            __at(2E82, int16 blood_material);
            __at(2E84, int32 blood_species);
            __at(2EDC, word pus_state);
            __at(2EDE, int16 pus_material);
            __at(2EE0, int32 pus_species);
            __at(2F38, Set<int16> egg_material);
            __at(2F48, Set<int32> egg_species);
            __at(2F88, Set<int16> unusual_egg_item_type);
            __at(2F98, Set<int16> unusual_egg_item_subtype);
            __at(2FA8, Set<int16> unusual_egg_material);
            __at(2FB8, Set<int32> unusual_egg_species);
            __at(3018, Set<Secretion *> secretion);
            __at(3028, Set<String *> creature_class);
            __at(3038, Set<String *> gobble_vermin_class);
            __at(3048, Set<String *> gobble_vermin_creature_token);
            __at(3058, Set<String *> gobble_vermin_caste_token);
            __at(31AC, Set<word> habit_id);
            __at(31BC, Set<dword> habit_probability);
            __at(31CC, Set<word> lair_type_id);
            __at(31DC, Set<dword> lair_type_probability);
            __at(31EC, Set<word> lair_characteristics_id);
            __at(31FC, Set<dword> lair_characteristics_probability);
            __at(322C, Set<int32> specific_food_creature_species);
            __at(323C, Set<int32> specific_food_plant_species);
            __at(326C, Set<CreatureSound *> sound);
            __at(329C, Set<MaterialForceMultiplier *> material_force_multiplier);
            __at(32AC, dword smell_trigger);
            __at(32B0, dword odor_level);
            __at(32B4, String odor_name);
            __at(32D0, dword low_light_vision);
            __at(32D4, Set<String *> sense_creature_class);
            __at(32E4, Set<byte> sense_creature_tile);
            __at(32F4, Set<word> sense_creature_color_fore);
            __at(3304, Set<word> sense_creature_color_back);
            __at(3314, Set<word> sense_creature_color_fore_bright);
        };

        struct Flags {
            dword
            equipment_wagon : 2,
            mundane : 1,
            vermin_eater : 1,
            vermin_grounder : 1,
            vermin_rotter : 1,
            vermin_soil : 1,
            vermin_soil_colony : 1,

            large_roaming : 1,
            vermin_fish : 1,
            loose_clusters : 1,
            fanciful : 1,
            unknown_12 : 1,
            unknown_13 : 1,
            unknown_14 : 1,
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
            subterranean_lava : 1,
            good : 1,

            evil : 1,
            savage : 1,
            unknown_66 : 1,
            unknown_67 : 1,
            has_male : 1,
            has_female : 1,
            unknown_70 : 1,
            unknown_71 : 1,

            unknown_72 : 1,
            unknown_73 : 1,
            generated_74 : 1,
            unknown_75 : 1,
            unknown_76 : 1,
            unknown_77 : 1,
            unknown_78 : 1,
            unknown_79 : 1,

            unknown_80 : 1,
            unknown_81 : 1,
            unknown_82 : 1,
            unknown_83 : 1,
            unknown_84 : 1,
            unknown_85 : 1,
            unknown_86 : 1,
            unknown_87 : 1,

            unknown_88 : 1,
            unknown_89 : 1,
            unknown_90 : 1,
            generated_91 : 1,
            unknown_92 : 1,
            unknown_93 : 1,
            does_not_exist : 1,
            unknown_95 : 1,

            unknown_96 : 1,
            unknown_97 : 1,
            unknown_98 : 1,
            unknown_99 : 1,
            unknown_100 : 1,
            unknown_101 : 1,
            unknown_102 : 1,
            artifact_hiveable : 1,

            ubiquitous : 1;
        };

        __at(0, String identifier);
        __at(1C, String name_singular);
        __at(38, String name_plural);
        __at(54, String adjective);
        __at(70, String baby_name_singular);
        __at(8C, String baby_name_plural);
        __at(A8, String child_name_singular);
        __at(C4, String child_name_plural);
        __at(E0, byte creature_tile);
        __at(E1, byte soldier_tile);
        __at(E2, byte creature_tile_alt);
        __at(E3, byte soldier_tile_alt);
        __at(E4, byte glow_tile);
        __at(EA, word frequency);
        __at(EC, word population_max);
        __at(EE, word population_min);
        __at(F0, word cluster_max);
        __at(F2, word cluster_min);
        __at(F4, word triggerable_group_max);
        __at(F6, word trigerrable_group_min);
        __at(F8, word color_fore);
        __at(FA, word color_back);
        __at(FC, word color_fore_bright);
        __at(FE, word glow_fore);
        __at(100, word glow_back);
        __at(102, word glow_fore_bright);
        __at(104, dword max_body_size); // max BODY_SIZE / 10
        __at(108, Set<String *> pref_strings);
        __at(118, Set<word> spheres);
        __at(128, Set<CasteRaw *> castes);
        __at(148, Flags * flags);
        __at(1DA4, Set<byte> speech_type);
        __at(1DD4, Set<MaterialRaw *> materials);
        __at(1DE4, Set<TissueRaw *> tissues);
        __at(1DF4, String profession_name_singular[111]);
        __at(2A18, String profession_name_plural[111]);
        __at(363C, dword undergroud_depth_min);
        __at(3640, dword undergroud_depth_max);
        __at(3664, Set<dword> hive_product_count);
        __at(3674, Set<dword> hive_product_time);
        __at(3684, Set<int16> hive_product_item_type);
        __at(3694, Set<int16> hive_product_item_subtype);
        __at(36A4, Set<int16> hive_product_material);
        __at(36B4, Set<int32> hive_product_species);
    };

    union Creature {
        struct Flags {
            dword
            unknown_0 : 1,
            dead_or_missing : 1,
            unknown_2 : 1,
            artifact_creator : 1,
            hostile_4 : 1,
            unknown_5 : 1,
            friendly_6 : 1,
            friendly_7 : 1,

            unknown_8 : 1,
            unknown_9 : 1,
            unknown_10 : 1,
            friendly_11 : 1,
            unknown_12 : 1,
            unknown_13 : 1,
            unknown_14 : 1,
            prone : 1,

            unknown_16 : 1, // cannot select
            invader_17 : 1,
            unknown_18 : 1,
            invader_19 : 1,
            unknown_20 : 1,
            unknown_21 : 1,
            unknown_22 : 1,
            unknown_23 : 1,

            unknown_24 : 1,
            caged : 1,
            pet : 1,
            chained : 1,
            unknown_28 : 1,
            unknown_29 : 1,
            unknown_30 : 1,
            unknown_31 : 1,

            unknown_32 : 1,
            unknown_33 : 1,
            unknown_34 : 1,
            unknown_35 : 1,
            unknown_36 : 1,
            unknown_37 : 1,
            unknown_38 : 1,
            unknown_39 : 1,

            unknown_40 : 1,
            unknown_41 : 1,
            unknown_42 : 1,
            unknown_43 : 1,
            unknown_44 : 1,
            unknown_45 : 1,
            unknown_46 : 1,
            unknown_47 : 1,

            unknown_48 : 1,
            unknown_49 : 1,
            hostile_50 : 1,
            hostile_51 : 1,
            unknown_52 : 1,
            unknown_53 : 1,
            hostile_54 : 1,
            visitor_55 : 1,

            unknown_56 : 1,
            unknown_57 : 1,
            unknown_58 : 1,
            unknown_59 : 1,
            unknown_60 : 1,
            unknown_61 : 1,
            unknown_62 : 1,
            unknown_63 : 1,

            unknown_64 : 1,
            unknown_65 : 1,
            unknown_66 : 1,
            unknown_67 : 1,
            unknown_68 : 1,
            unknown_69 : 1,
            unknown_70 : 1,
            unknown_71 : 1,

            unknown_72 : 1,
            unknown_73 : 1,
            unknown_74 : 1,
            unknown_75 : 1,
            ghostly : 1,
            unknown_77 : 1,
            unknown_78 : 1,
            unknown_79 : 1,

            unknown_80 : 1,
            unknown_81 : 1,
            unknown_82 : 1,
            unknown_83 : 1,
            unknown_84 : 1,
            unknown_85 : 1,
            unknown_86 : 1,
            unknown_87 : 1,

            unknown_88 : 1,
            emotionally_overloaded : 1,
            unknown_90 : 1,
            unknown_91 : 1,
            unknown_92 : 1,
            unknown_93 : 1,
            unknown_94 : 1,
            unknown_95 : 1;
        };

        __at(0, String name);
        __at(1C, String nickname);
        __at(38, dword surname_1_index); // or middle name if surname_3 is present
        __at(3C, dword surname_2_index); // or middle name if surname_3 is present
        __at(4C, dword surname_3_index);
        __at(64, dword surname_language);
        __at(6A, byte has_name);
        __at(6C, String custom_profession_name);
        __at(88, word profession);
        __at(8C, word species);
        __at(90, Vector3<int16> location);
        __at(A8, Vector3<int16> destination);
        __at(E0, Flags flags);
        __at(F8, word caste);
        __at(FA, byte sex);
        __at(10C, dword civilization);
        __at(468, dword strength);
        __at(484, dword agility);
        __at(4A0, dword toughness);
        __at(4BC, dword endurance);
        __at(4D8, dword recuperation);
        __at(4F4, dword disease_resistance);
    };
}

#endif

#endif

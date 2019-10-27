#pragma once

#include "TestSpellHunter.h"

class AimedShot;
class MultiShot;

class TestAimedShot: public TestSpellHunter {
public:
    TestAimedShot(EquipmentDb *equipment_db);

    void test_all();

private:
    AimedShot* aimed_shot() const;
    MultiShot* multi_shot() const;

    void test_name_correct() override;
    void test_spell_cooldown() override;
    void test_whether_spell_causes_global_cooldown() override;
    void test_how_spell_observes_global_cooldown() override;
    void test_resource_cost() override;
    void test_is_ready_conditions() override;

    void test_dmg_affected_by_projectile_bonus();
    void test_hit_dmg_0_of_5_ranged_weapon_specialization();
    void test_hit_dmg_5_of_5_ranged_weapon_specialization();
    void test_crit_dmg_0_of_5_mortal_shots_0_of_5_slaying();
    void test_crit_dmg_1_of_5_mortal_shots_0_of_5_slaying();
    void test_crit_dmg_2_of_5_mortal_shots_0_of_5_slaying();
    void test_crit_dmg_3_of_5_mortal_shots_0_of_5_slaying();
    void test_crit_dmg_4_of_5_mortal_shots_0_of_5_slaying();
    void test_crit_dmg_5_of_5_mortal_shots_0_of_5_slaying();
    void test_crit_dmg_5_of_5_mortal_shots_1_of_3_monster_slaying();
    void test_crit_dmg_5_of_5_mortal_shots_2_of_3_monster_slaying();
    void test_crit_dmg_5_of_5_mortal_shots_3_of_3_monster_slaying();
    void test_crit_dmg_5_of_5_mortal_shots_1_of_3_humanoid_slaying();
    void test_crit_dmg_5_of_5_mortal_shots_2_of_3_humanoid_slaying();
    void test_crit_dmg_5_of_5_mortal_shots_3_of_3_humanoid_slaying();

    void test_aimed_shot_adds_player_action_event_on_completion();
    void test_aimed_shot_cast_in_progress_blocks_other_spells();

    void test_mana_cost_1_of_5_efficiency();
    void test_mana_cost_2_of_5_efficiency();
    void test_mana_cost_3_of_5_efficiency();
    void test_mana_cost_4_of_5_efficiency();
    void test_mana_cost_5_of_5_efficiency();

    void test_aimed_shot_cast_time_not_reduced_by_ranged_attack_speed_boosts();

    void given_aimed_shot_is_enabled();
    void when_aimed_shot_is_performed();
};

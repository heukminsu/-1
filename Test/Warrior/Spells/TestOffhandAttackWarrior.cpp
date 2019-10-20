#include "TestOffhandAttackWarrior.h"

#include <cassert>

#include "Equipment.h"
#include "MainhandAttackWarrior.h"
#include "OffhandAttackWarrior.h"
#include "Talent.h"

TestOffhandAttackWarrior::TestOffhandAttackWarrior(EquipmentDb *equipment_db) :
    TestSpellWarrior(equipment_db, "OffhandAttackWarrior")
{}

void TestOffhandAttackWarrior::test_all() {
    run_mandatory_tests();

    set_up();
    test_changing_weapons_changes_cooldown();
    tear_down();

    set_up();
    test_hit_dmg_0_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_hit_dmg_1_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_hit_dmg_2_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_hit_dmg_3_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_hit_dmg_4_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_hit_dmg_5_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_crit_dmg_0_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_crit_dmg_1_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_crit_dmg_2_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_crit_dmg_3_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_crit_dmg_4_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_crit_dmg_5_of_5_dual_wield_specialization();
    tear_down();

    set_up();
    test_glancing_damage_300_wpn_skill();
    tear_down();

    set_up();
    test_glancing_damage_305_wpn_skill();
    tear_down();

    set_up();
    test_glancing_damage_310_wpn_skill();
    tear_down();

    set_up();
    test_glancing_damage_315_wpn_skill();
    tear_down();

    set_up();
    test_mid_swing_haste_increase_updates_attack_speed();
    tear_down();

    set_up();
    test_mid_swing_haste_decrease_updates_attack_speed();
    tear_down();

    set_up();
    test_dodge_applies_overpower_buff();
    tear_down();
}

OffhandAttackWarrior* TestOffhandAttackWarrior::oh_attack() const {
    auto* spells = dynamic_cast<WarriorSpells*>(warrior->get_spells());
    return dynamic_cast<OffhandAttackWarrior*>(spells->get_oh_attack());
}

void TestOffhandAttackWarrior::test_name_correct() {
    assert(oh_attack()->get_name() == "Offhand Attack");
}

void TestOffhandAttackWarrior::test_spell_cooldown() {
    given_an_offhand_weapon_with_3_speed();

    when_oh_attack_is_performed();

    then_next_expected_use_is(3.0);
}

void TestOffhandAttackWarrior::test_how_spell_observes_global_cooldown() {
    assert(oh_attack()->get_spell_status() == SpellStatus::Available);

    given_warrior_is_on_gcd();

    assert(oh_attack()->get_spell_status() == SpellStatus::Available);
}

void TestOffhandAttackWarrior::test_resource_cost() {
    given_warrior_has_rage(0);
    assert(oh_attack()->get_spell_status() == SpellStatus::Available);
}

void TestOffhandAttackWarrior::test_is_ready_conditions() {
    // No conditions not tested in other mandatory tests.
}

void TestOffhandAttackWarrior::test_stance_cooldown() {
    when_switching_to_berserker_stance();

    assert(warrior->on_stance_cooldown() == true);

    assert(oh_attack()->get_spell_status() == SpellStatus::Available);
}

void TestOffhandAttackWarrior::test_changing_weapons_changes_cooldown() {
    given_an_offhand_weapon_with_3_speed();
    when_oh_attack_is_performed();
    then_next_expected_use_is(3.0);

    when_changing_to_2_speed();
    when_oh_attack_is_performed();
    then_next_expected_use_is(2.0);
}

void TestOffhandAttackWarrior::test_whether_spell_causes_global_cooldown() {
    assert(warrior->action_ready());

    when_oh_attack_is_performed();

    assert(warrior->action_ready());
}

void TestOffhandAttackWarrior::test_hit_dmg_0_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_hit();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * dual_wield_penalty
    // [143] = (100 + (2.6 * 1000 / 14)) * 0.5
    then_damage_dealt_is(143);
}

void TestOffhandAttackWarrior::test_hit_dmg_1_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_hit();
    given_fury_talent_with_rank("Dual Wield Specialization", 1);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * dual_wield_penalty
    // [150] = (100 + (2.6 * 1000 / 14)) * 0.525
    then_damage_dealt_is(150);
}

void TestOffhandAttackWarrior::test_hit_dmg_2_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_hit();
    given_fury_talent_with_rank("Dual Wield Specialization", 2);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * dual_wield_penalty
    // [157] = (100 + (2.6 * 1000 / 14)) * 0.55
    then_damage_dealt_is(157);
}

void TestOffhandAttackWarrior::test_hit_dmg_3_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_hit();
    given_fury_talent_with_rank("Dual Wield Specialization", 3);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * dual_wield_penalty
    // [164] = (100 + (2.6 * 1000 / 14)) * 0.575
    then_damage_dealt_is(164);
}

void TestOffhandAttackWarrior::test_hit_dmg_4_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_hit();
    given_fury_talent_with_rank("Dual Wield Specialization", 4);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * dual_wield_penalty
    // [171] = (100 + (2.6 * 1000 / 14)) * 0.6
    then_damage_dealt_is(171);
}

void TestOffhandAttackWarrior::test_hit_dmg_5_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_hit();
    given_fury_talent_with_rank("Dual Wield Specialization", 5);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * dual_wield_penalty
    // [179] = (100 + (2.6 * 1000 / 14)) * 0.625
    then_damage_dealt_is(179);
}

void TestOffhandAttackWarrior::test_crit_dmg_0_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_crit();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * crit_dmg_modifier * dual_wield_penalty
    // [286] = (100 + (2.6 * 1000 / 14)) * 2 * 0.5
    then_damage_dealt_is(286);
}

void TestOffhandAttackWarrior::test_crit_dmg_1_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_crit();
    given_fury_talent_with_rank("Dual Wield Specialization", 1);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * crit_dmg_modifier* dual_wield_penalty
    // [300] = (100 + (2.6 * 1000 / 14)) * 2 * 0.525
    then_damage_dealt_is(300);
}

void TestOffhandAttackWarrior::test_crit_dmg_2_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_crit();
    given_fury_talent_with_rank("Dual Wield Specialization", 2);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * crit_dmg_modifier * dual_wield_penalty
    // [314] = (100 + (2.6 * 1000 / 14)) * 2 * 0.55
    then_damage_dealt_is(314);
}

void TestOffhandAttackWarrior::test_crit_dmg_3_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_crit();
    given_fury_talent_with_rank("Dual Wield Specialization", 3);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * crit_dmg_modifier* dual_wield_penalty
    // [329] = (100 + (2.6 * 1000 / 14)) * 2 * 0.575
    then_damage_dealt_is(329);
}

void TestOffhandAttackWarrior::test_crit_dmg_4_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_crit();
    given_fury_talent_with_rank("Dual Wield Specialization", 4);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * crit_dmg_modifier * dual_wield_penalty
    // [343] = (100 + (2.6 * 1000 / 14)) * 2 * 0.6
    then_damage_dealt_is(343);
}

void TestOffhandAttackWarrior::test_crit_dmg_5_of_5_dual_wield_specialization() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_crit();
    given_fury_talent_with_rank("Dual Wield Specialization", 5);
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * crit_dmg_modifier * dual_wield_penalty
    // [357] = (100 + (2.6 * 1000 / 14)) * 2 * 0.625
    then_damage_dealt_is(357);
}

void TestOffhandAttackWarrior::test_glancing_damage_300_wpn_skill() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_glancing_blow();
    given_300_weapon_skill_oh();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * glancing_dmg_modifier * dual_wield_penalty
    // [79 - 107] = (100 + (2.6 * 1000 / 14)) * [0.55 - 0.75] * 0.5
    then_damage_dealt_is_in_range(79, 107);
}

void TestOffhandAttackWarrior::test_glancing_damage_305_wpn_skill() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_305_weapon_skill_oh();
    given_a_guaranteed_white_glancing_blow();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * glancing_dmg_modifier * dual_wield_penalty
    // [114 - 129] = (100 + (2.6 * 1000 / 14)) * [0.8 - 0.9] * 0.5
    then_damage_dealt_is_in_range(114, 129);
}

void TestOffhandAttackWarrior::test_glancing_damage_310_wpn_skill() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_310_weapon_skill_oh();
    given_a_guaranteed_white_glancing_blow();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * glancing_dmg_modifier * dual_wield_penalty
    // [130 - 141] = (100 + (2.6 * 1000 / 14)) * [0.91 - 0.99] * 0.5
    then_damage_dealt_is_in_range(130, 141);
}

void TestOffhandAttackWarrior::test_glancing_damage_315_wpn_skill() {
    given_target_has_0_armor();
    given_an_offhand_weapon_with_100_min_max_dmg();
    given_315_weapon_skill_oh();
    given_a_guaranteed_white_glancing_blow();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();
    given_2_of_2_impale();

    when_oh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * glancing_dmg_modifier * dual_wield_penalty
    // [130 - 141] = (100 + (2.6 * 1000 / 14)) * [0.91 - 0.99] * 0.5
    then_damage_dealt_is_in_range(130, 141);
}

void TestOffhandAttackWarrior::test_mid_swing_haste_increase_updates_attack_speed() {
    given_a_mainhand_weapon_with_3_speed();
    given_an_offhand_weapon_with_2_speed();
    when_mh_attack_is_performed();
    when_oh_attack_is_performed();
    then_next_expected_use_is(2.0);
    given_engine_priority_at(1.0);

    when_increasing_attack_speed(100);

    then_next_expected_use_is(1.5);
}

void TestOffhandAttackWarrior::test_mid_swing_haste_decrease_updates_attack_speed() {
    given_a_mainhand_weapon_with_3_speed();
    given_an_offhand_weapon_with_3_speed();
    when_increasing_attack_speed(100);
    when_mh_attack_is_performed();
    when_oh_attack_is_performed();
    given_engine_priority_at(1.0);

    when_decreasing_attack_speed(100);

    then_next_expected_use_is(2.0);
}

void TestOffhandAttackWarrior::test_dodge_applies_overpower_buff() {
    given_a_guaranteed_white_dodge();

    when_oh_attack_is_performed();

    then_overpower_is_active();
}

void TestOffhandAttackWarrior::when_mh_attack_is_performed() {
    warrior->get_spells()->get_mh_attack()->perform();
}

void TestOffhandAttackWarrior::when_oh_attack_is_performed() {
    if (pchar->get_equipment()->get_offhand() == nullptr)
        given_an_offhand_weapon_with_100_min_max_dmg();

    oh_attack()->perform();
}

void TestOffhandAttackWarrior::when_changing_to_2_speed() {
    given_an_offhand_weapon_with_2_speed();
}

void TestOffhandAttackWarrior::when_increasing_attack_speed(const unsigned change) {
    warrior->increase_melee_attack_speed(change);
}

void TestOffhandAttackWarrior::when_decreasing_attack_speed(const unsigned change) {
    warrior->decrease_melee_attack_speed(change);
}

void TestOffhandAttackWarrior::then_next_expected_use_is(const double next_expected_use) {
    assert(QString::number(oh_attack()->get_next_expected_use(), 'f', 3) == QString::number(next_expected_use, 'f', 3));
}

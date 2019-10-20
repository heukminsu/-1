#include "TestMainhandAttackPaladin.h"

#include <cassert>

#include "MainhandAttackPaladin.h"
#include "Paladin.h"

TestMainhandAttackPaladin::TestMainhandAttackPaladin(EquipmentDb *equipment_db) :
    TestSpellPaladin(equipment_db, "MainhandAttackPaladin")
{}

void TestMainhandAttackPaladin::test_all() {
    run_mandatory_tests();

    set_up();
    test_changing_weapons_changes_cooldown();
    tear_down();

    set_up();
    test_hit_dmg();
    tear_down();

    set_up();
    test_crit_dmg();
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
    test_seal_of_the_crusader_lowers_damage();
    tear_down();
}

void TestMainhandAttackPaladin::test_name_correct() {
    assert(mh_attack()->get_name() == "Mainhand Attack");
}

void TestMainhandAttackPaladin::test_spell_cooldown() {
    given_a_mainhand_weapon_with_3_speed();

    when_mh_attack_is_performed();

    then_next_expected_use_is(3.0);
}

void TestMainhandAttackPaladin::test_how_spell_observes_global_cooldown() {
    assert(mh_attack()->get_spell_status() == SpellStatus::Available);

    given_paladin_is_on_gcd();

    assert(mh_attack()->get_spell_status() == SpellStatus::Available);
}

void TestMainhandAttackPaladin::test_resource_cost() {
    given_paladin_has_mana(0);
    assert(mh_attack()->get_spell_status() == SpellStatus::Available);
}

void TestMainhandAttackPaladin::test_is_ready_conditions() {
    // No conditions not tested in other mandatory tests.
}

void TestMainhandAttackPaladin::test_changing_weapons_changes_cooldown() {
    given_a_mainhand_weapon_with_3_speed();
    when_mh_attack_is_performed();
    then_next_expected_use_is(3.0);

    when_changing_to_2_speed();
    when_mh_attack_is_performed();
    then_next_expected_use_is(2.0);
}

void TestMainhandAttackPaladin::test_whether_spell_causes_global_cooldown() {
    assert(paladin->action_ready());

    when_mh_attack_is_performed();

    assert(paladin->action_ready());
}

void TestMainhandAttackPaladin::test_hit_dmg() {
    given_target_has_0_armor();
    given_a_mainhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_hit();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();

    when_mh_attack_is_performed();

    // [Damage] = base_dmg + (wpn_speed * AP / 14)
    // [286] = 100 + (2.6 * 1000 / 14)
    then_damage_dealt_is(286);
}

void TestMainhandAttackPaladin::test_crit_dmg() {
    given_target_has_0_armor();
    given_a_mainhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_crit();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();

    when_mh_attack_is_performed();

    // [Damage] = base_dmg + (wpn_speed * AP / 14) * crit_dmg_modifier
    // [571] = (100 + (2.6 * 1000 / 14)) * 2.0
    then_damage_dealt_is(571);
}

void TestMainhandAttackPaladin::test_glancing_damage_300_wpn_skill() {
    given_target_has_0_armor();
    given_a_mainhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_glancing_blow();
    given_300_weapon_skill_mh();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();

    when_mh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * glancing_dmg_modifier
    // [157 - 214] = (100 + (2.6 * 1000 / 14)) * [0.55 - 0.75]
    then_damage_dealt_is_in_range(157, 214);
}

void TestMainhandAttackPaladin::test_glancing_damage_305_wpn_skill() {
    given_target_has_0_armor();
    given_a_mainhand_weapon_with_100_min_max_dmg();
    given_305_weapon_skill_mh();
    given_a_guaranteed_white_glancing_blow();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();

    when_mh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * glancing_dmg_modifier
    // [229 - 257] = (100 + (2.6 * 1000 / 14)) * [0.8 - 0.9]
    then_damage_dealt_is_in_range(229, 257);
}

void TestMainhandAttackPaladin::test_glancing_damage_310_wpn_skill() {
    given_target_has_0_armor();
    given_a_mainhand_weapon_with_100_min_max_dmg();
    given_310_weapon_skill_mh();
    given_a_guaranteed_white_glancing_blow();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();

    when_mh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * glancing_dmg_modifier
    // [260 - 283] = (100 + (2.6 * 1000 / 14)) * [0.91 - 0.99]
    then_damage_dealt_is_in_range(260, 283);
}

void TestMainhandAttackPaladin::test_glancing_damage_315_wpn_skill() {
    given_target_has_0_armor();
    given_a_mainhand_weapon_with_100_min_max_dmg();
    given_315_weapon_skill_mh();
    given_a_guaranteed_white_glancing_blow();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();

    when_mh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * glancing_dmg_modifier
    // [260 - 283] = (100 + (2.6 * 1000 / 14)) * [0.91 - 0.99]
    then_damage_dealt_is_in_range(260, 283);
}

void TestMainhandAttackPaladin::test_mid_swing_haste_increase_updates_attack_speed() {
    given_a_mainhand_weapon_with_2_speed();
    given_no_offhand();
    when_mh_attack_is_performed();
    then_next_expected_use_is(2.0);
    given_engine_priority_at(1.0);

    when_increasing_attack_speed(100);

    then_next_expected_use_is(1.5);
}

void TestMainhandAttackPaladin::test_mid_swing_haste_decrease_updates_attack_speed() {
    given_a_mainhand_weapon_with_3_speed();
    given_no_offhand();
    when_increasing_attack_speed(100);
    when_mh_attack_is_performed();
    then_next_expected_use_is(1.5);
    given_engine_priority_at(1.0);

    when_decreasing_attack_speed(100);

    then_next_expected_use_is(2.0);
}

void TestMainhandAttackPaladin::test_seal_of_the_crusader_lowers_damage() {
    given_target_has_0_armor();
    given_a_mainhand_weapon_with_100_min_max_dmg();
    given_a_guaranteed_white_hit();
    given_seal_of_the_crusader_is_active();
    given_1000_melee_ap();
    given_no_previous_damage_dealt();

    when_mh_attack_is_performed();

    // [Damage] = (base_dmg + (wpn_speed * AP / 14)) * sotc_penalty
    // [204] = (100 + (2.6 * 1000 / 14)) * (1 / 1.4)
    then_damage_dealt_is(204);
}

void TestMainhandAttackPaladin::when_changing_to_2_speed() {
    given_a_mainhand_weapon_with_2_speed();
}

void TestMainhandAttackPaladin::when_increasing_attack_speed(const int change) {
    paladin->increase_melee_attack_speed(change);
}

void TestMainhandAttackPaladin::when_decreasing_attack_speed(const int change) {
    paladin->decrease_melee_attack_speed(change);
}

void TestMainhandAttackPaladin::then_next_expected_use_is(const double next_expected_use) {
    assert(QString::number(mh_attack()->get_next_expected_use(), 'f', 3) == QString::number(next_expected_use, 'f', 3));
}

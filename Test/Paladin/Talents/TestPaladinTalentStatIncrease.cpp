#include "TestPaladinTalentStatIncrease.h"

#include <cassert>

#include "CharacterStats.h"
#include "HolyPaladin.h"
#include "Paladin.h"
#include "ProtectionPaladin.h"
#include "Retribution.h"
#include "Talent.h"

TestPaladinTalentStatIncrease::TestPaladinTalentStatIncrease(EquipmentDb* equipment_db) :
    TestSpellPaladin(equipment_db, "PaladinTalentStatIncrease")
{}

void TestPaladinTalentStatIncrease::test_all() {
    run_mandatory_tests();

    set_up();
    test_conviction();
    tear_down();

    set_up();
    test_two_handed_weapon_specialization();
    tear_down();

    set_up();
    test_precision();
    tear_down();

    set_up();
    test_one_handed_weapon_specialization();
    tear_down();

    set_up();
    test_divine_strength();
    tear_down();

    set_up();
    test_divine_intellect();
    tear_down();
}

void TestPaladinTalentStatIncrease::test_conviction() {
    auto ret = Retribution(paladin);
    Talent* talent = ret.get_talent_from_name("Conviction");

    unsigned melee_crit_initial = pchar->get_stats()->get_mh_crit_chance();
    unsigned ranged_crit_initial = pchar->get_stats()->get_ranged_crit_chance();

    assert(talent->increment_rank());
    assert(melee_crit_initial + suppressed_aura_crit(100) == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial + suppressed_aura_crit(100) == pchar->get_stats()->get_ranged_crit_chance());

    assert(talent->increment_rank());
    assert(melee_crit_initial + suppressed_aura_crit(200) == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial + suppressed_aura_crit(200) == pchar->get_stats()->get_ranged_crit_chance());

    assert(talent->increment_rank());
    assert(melee_crit_initial + suppressed_aura_crit(300) == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial + suppressed_aura_crit(300) == pchar->get_stats()->get_ranged_crit_chance());

    assert(talent->increment_rank());
    assert(melee_crit_initial + suppressed_aura_crit(400) == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial + suppressed_aura_crit(400) == pchar->get_stats()->get_ranged_crit_chance());

    assert(talent->increment_rank());
    assert(melee_crit_initial + suppressed_aura_crit(500) == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial + suppressed_aura_crit(500) == pchar->get_stats()->get_ranged_crit_chance());

    assert(talent->decrement_rank());
    assert(melee_crit_initial + suppressed_aura_crit(400) == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial + suppressed_aura_crit(400) == pchar->get_stats()->get_ranged_crit_chance());

    assert(talent->decrement_rank());
    assert(melee_crit_initial + suppressed_aura_crit(300) == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial + suppressed_aura_crit(300) == pchar->get_stats()->get_ranged_crit_chance());

    assert(talent->decrement_rank());
    assert(melee_crit_initial + suppressed_aura_crit(200) == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial + suppressed_aura_crit(200) == pchar->get_stats()->get_ranged_crit_chance());

    assert(talent->decrement_rank());
    assert(melee_crit_initial + suppressed_aura_crit(100) == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial + suppressed_aura_crit(100) == pchar->get_stats()->get_ranged_crit_chance());

    assert(talent->decrement_rank());
    assert(melee_crit_initial == pchar->get_stats()->get_mh_crit_chance());
    assert(ranged_crit_initial == pchar->get_stats()->get_ranged_crit_chance());
}

void TestPaladinTalentStatIncrease::test_two_handed_weapon_specialization() {
    auto ret = Retribution(paladin);
    Talent* talent = ret.get_talent_from_name("Two-Handed Weapon Specialization");
    given_a_twohand_weapon_with_100_min_max_dmg();

    assert(almost_equal(1.0, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->increment_rank());
    assert(almost_equal(1.02, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->increment_rank());
    assert(almost_equal(1.04, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->increment_rank());
    assert(almost_equal(1.06, paladin->get_stats()->get_total_physical_damage_mod()));

    given_1h_mace_equipped_in_mainhand(paladin);
    assert(almost_equal(1.0, paladin->get_stats()->get_total_physical_damage_mod()));

    given_a_twohand_weapon_with_100_min_max_dmg();
    assert(almost_equal(1.06, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->decrement_rank());
    assert(almost_equal(1.04, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->decrement_rank());
    assert(almost_equal(1.02, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->decrement_rank());
    assert(almost_equal(1.0, paladin->get_stats()->get_total_physical_damage_mod()));
}

void TestPaladinTalentStatIncrease::test_precision() {
    auto prot = ProtectionPaladin(paladin);
    Talent* talent = prot.get_talent_from_name("Precision");

    unsigned melee_hit_initial = pchar->get_stats()->get_melee_hit_chance();
    unsigned ranged_hit_initial = pchar->get_stats()->get_ranged_hit_chance();

    assert(talent->increment_rank());
    assert(melee_hit_initial + 100 == pchar->get_stats()->get_melee_hit_chance());
    assert(ranged_hit_initial + 100 == pchar->get_stats()->get_ranged_hit_chance());

    assert(talent->increment_rank());
    assert(melee_hit_initial + 200 == pchar->get_stats()->get_melee_hit_chance());
    assert(ranged_hit_initial + 200 == pchar->get_stats()->get_ranged_hit_chance());

    assert(talent->increment_rank());
    assert(melee_hit_initial + 300 == pchar->get_stats()->get_melee_hit_chance());
    assert(ranged_hit_initial + 300 == pchar->get_stats()->get_ranged_hit_chance());

    assert(talent->decrement_rank());
    assert(melee_hit_initial + 200 == pchar->get_stats()->get_melee_hit_chance());
    assert(ranged_hit_initial + 200 == pchar->get_stats()->get_ranged_hit_chance());

    assert(talent->decrement_rank());
    assert(melee_hit_initial + 100 == pchar->get_stats()->get_melee_hit_chance());
    assert(ranged_hit_initial + 100 == pchar->get_stats()->get_ranged_hit_chance());

    assert(talent->decrement_rank());
    assert(melee_hit_initial == pchar->get_stats()->get_melee_hit_chance());
    assert(ranged_hit_initial == pchar->get_stats()->get_ranged_hit_chance());
}

void TestPaladinTalentStatIncrease::test_one_handed_weapon_specialization() {
    auto prot = ProtectionPaladin(paladin);
    Talent* talent = prot.get_talent_from_name("One-Handed Weapon Specialization");
    given_1h_mace_equipped_in_mainhand(paladin);

    assert(almost_equal(1.0, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->increment_rank());
    assert(almost_equal(1.02, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->increment_rank());
    assert(almost_equal(1.04, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->increment_rank());
    assert(almost_equal(1.06, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->increment_rank());
    assert(almost_equal(1.08, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->increment_rank());
    assert(almost_equal(1.10, paladin->get_stats()->get_total_physical_damage_mod()));

    given_a_twohand_weapon_with_100_min_max_dmg();
    assert(almost_equal(1.0, paladin->get_stats()->get_total_physical_damage_mod()));

    given_1h_mace_equipped_in_mainhand(paladin);
    assert(almost_equal(1.10, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->decrement_rank());
    assert(almost_equal(1.08, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->decrement_rank());
    assert(almost_equal(1.06, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->decrement_rank());
    assert(almost_equal(1.04, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->decrement_rank());
    assert(almost_equal(1.02, paladin->get_stats()->get_total_physical_damage_mod()));

    assert(talent->decrement_rank());
    assert(almost_equal(1.0, paladin->get_stats()->get_total_physical_damage_mod()));
}

void TestPaladinTalentStatIncrease::test_divine_strength() {
    auto holy = HolyPaladin(paladin);
    Talent* talent = holy.get_talent_from_name("Divine Strength");
    given_character_has_strength(100);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_strength() == 102);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_strength() == 104);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_strength() == 106);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_strength() == 108);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_strength() == 110);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_strength() == 108);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_strength() == 106);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_strength() == 104);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_strength() == 102);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_strength() == 100);
}

void TestPaladinTalentStatIncrease::test_divine_intellect() {
    auto holy = HolyPaladin(paladin);
    Talent* talent = holy.get_talent_from_name("Divine Intellect");
    given_character_has_intellect(100);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_intellect() == 102);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_intellect() == 104);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_intellect() == 106);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_intellect() == 108);

    assert(talent->increment_rank());
    assert(pchar->get_stats()->get_intellect() == 110);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_intellect() == 108);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_intellect() == 106);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_intellect() == 104);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_intellect() == 102);

    assert(talent->decrement_rank());
    assert(pchar->get_stats()->get_intellect() == 100);
}

void TestPaladinTalentStatIncrease::test_name_correct() {
}

void TestPaladinTalentStatIncrease::test_spell_cooldown() {
}

void TestPaladinTalentStatIncrease::test_how_spell_observes_global_cooldown() {
}

void TestPaladinTalentStatIncrease::test_resource_cost() {
}

void TestPaladinTalentStatIncrease::test_is_ready_conditions() {
}

void TestPaladinTalentStatIncrease::test_whether_spell_causes_global_cooldown() {
}

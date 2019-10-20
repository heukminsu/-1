#include "TestBerserkerStance.h"

#include <cassert>

#include "BerserkerStance.h"
#include "CharacterStats.h"
#include "Event.h"

TestBerserkerStance::TestBerserkerStance(EquipmentDb *equipment_db) :
    TestSpellWarrior(equipment_db, "Berserker Stance")
{}

void TestBerserkerStance::test_all() {
    run_mandatory_tests();

    set_up();
    test_does_not_incur_extra_global_cooldown_if_gcd_longer_than_half_second();
    tear_down();

    set_up();
    test_gives_crit_when_stance_entered();
    tear_down();

    set_up();
    test_removes_crit_when_stance_exited();
    tear_down();

    set_up();
    test_0_rage_remains_after_stance_switch_with_0_of_5_tactical_mastery();
    tear_down();

    set_up();
    test_5_rage_remains_after_stance_switch_with_1_of_5_tactical_mastery();
    tear_down();

    set_up();
    test_10_rage_remains_after_stance_switch_with_2_of_5_tactical_mastery();
    tear_down();

    set_up();
    test_15_rage_remains_after_stance_switch_with_3_of_5_tactical_mastery();
    tear_down();

    set_up();
    test_20_rage_remains_after_stance_switch_with_4_of_5_tactical_mastery();
    tear_down();

    set_up();
    test_25_rage_remains_after_stance_switch_with_5_of_5_tactical_mastery();
    tear_down();

    set_up();
    test_rage_is_not_increased_by_switching_stances_with_5_of_5_tactical_mastery();
    tear_down();
}

BerserkerStance* TestBerserkerStance::berserker_stance() const {
    return dynamic_cast<WarriorSpells*>(warrior->get_spells())->get_berserker_stance();
}

void TestBerserkerStance::test_name_correct() {
    assert(berserker_stance()->get_name() == "Berserker Stance");
}

void TestBerserkerStance::test_spell_cooldown() {
    assert(QString::number(berserker_stance()->get_base_cooldown(), 'f', 3) == "0.000");
}

void TestBerserkerStance::test_stance_cooldown() {
    assert(warrior->on_stance_cooldown() == false);

    when_berserker_stance_is_performed();

    assert(warrior->on_stance_cooldown() == true);
    given_engine_priority_at(0.99);
    assert(warrior->on_stance_cooldown() == true);
    given_engine_priority_at(1.01);
    assert(warrior->on_stance_cooldown() == false);
}

void TestBerserkerStance::test_resource_cost() {
    warrior->lose_rage(warrior->get_resource_level(ResourceType::Rage));
    assert(berserker_stance()->get_spell_status() == SpellStatus::Available);
}

void TestBerserkerStance::test_is_ready_conditions() {
    assert(!warrior->on_global_cooldown());
    assert(!warrior->on_stance_cooldown());

    assert(berserker_stance()->get_spell_status() == SpellStatus::Available);
}

void TestBerserkerStance::test_whether_spell_causes_global_cooldown() {
    assert(warrior->on_global_cooldown() == false);

    when_berserker_stance_is_performed();

    assert(warrior->on_global_cooldown());
    given_engine_priority_at(0.49);
    assert(warrior->on_global_cooldown());
    given_engine_priority_at(0.51);
    assert(warrior->on_global_cooldown() == false);
}

void TestBerserkerStance::test_how_spell_observes_global_cooldown() {
    assert(berserker_stance()->get_spell_status() == SpellStatus::Available);

    given_warrior_is_on_gcd();

    assert(berserker_stance()->get_spell_status() == SpellStatus::OnGCD);
}

void TestBerserkerStance::test_does_not_incur_extra_global_cooldown_if_gcd_longer_than_half_second() {
    given_warrior_is_on_gcd();
    assert(warrior->on_stance_cooldown() == false);

    when_berserker_stance_is_performed();

    assert(warrior->on_global_cooldown());
    assert(warrior->on_stance_cooldown());
    given_engine_priority_at(0.99);
    assert(warrior->on_global_cooldown());
    assert(warrior->on_stance_cooldown());
    given_engine_priority_at(1.01);
    assert(warrior->on_global_cooldown());
    assert(warrior->on_stance_cooldown() == false);
    given_engine_priority_at(1.49);
    assert(warrior->on_global_cooldown());
    given_engine_priority_at(1.51);
    assert(warrior->on_global_cooldown() == false);
}

void TestBerserkerStance::test_gives_crit_when_stance_entered() {
    const unsigned crit_chance = pchar->get_stats()->get_mh_crit_chance();

    when_berserker_stance_is_performed();

    assert(pchar->get_stats()->get_mh_crit_chance() == crit_chance + suppressed_aura_crit(300));
}

void TestBerserkerStance::test_removes_crit_when_stance_exited() {
    const unsigned crit_chance = pchar->get_stats()->get_mh_crit_chance();

    when_berserker_stance_is_performed();
    assert(pchar->get_stats()->get_mh_crit_chance() == crit_chance + suppressed_aura_crit(300));

    dynamic_cast<Warrior*>(pchar)->switch_to_battle_stance();

    assert(pchar->get_stats()->get_mh_crit_chance() == crit_chance);
}

void TestBerserkerStance::test_0_rage_remains_after_stance_switch_with_0_of_5_tactical_mastery() {
    given_0_of_5_tactical_mastery();
    given_warrior_has_rage(100);

    when_berserker_stance_is_performed();

    then_warrior_has_rage(0);
}

void TestBerserkerStance::test_5_rage_remains_after_stance_switch_with_1_of_5_tactical_mastery() {
    given_1_of_5_tactical_mastery();
    given_warrior_has_rage(100);

    when_berserker_stance_is_performed();

    then_warrior_has_rage(5);
}

void TestBerserkerStance::test_10_rage_remains_after_stance_switch_with_2_of_5_tactical_mastery() {
    given_2_of_5_tactical_mastery();
    given_warrior_has_rage(100);

    when_berserker_stance_is_performed();

    then_warrior_has_rage(10);
}

void TestBerserkerStance::test_15_rage_remains_after_stance_switch_with_3_of_5_tactical_mastery() {
    given_3_of_5_tactical_mastery();
    given_warrior_has_rage(100);

    when_berserker_stance_is_performed();

    then_warrior_has_rage(15);
}

void TestBerserkerStance::test_20_rage_remains_after_stance_switch_with_4_of_5_tactical_mastery() {
    given_4_of_5_tactical_mastery();
    given_warrior_has_rage(100);

    when_berserker_stance_is_performed();

    then_warrior_has_rage(20);
}

void TestBerserkerStance::test_25_rage_remains_after_stance_switch_with_5_of_5_tactical_mastery() {
    given_5_of_5_tactical_mastery();
    given_warrior_has_rage(100);

    when_berserker_stance_is_performed();

    then_warrior_has_rage(25);
}

void TestBerserkerStance::test_rage_is_not_increased_by_switching_stances_with_5_of_5_tactical_mastery() {
    given_5_of_5_tactical_mastery();
    given_warrior_has_rage(0);

    when_berserker_stance_is_performed();

    then_warrior_has_rage(0);
}

void TestBerserkerStance::when_berserker_stance_is_performed() {
    dynamic_cast<Warrior*>(pchar)->switch_to_berserker_stance();
}

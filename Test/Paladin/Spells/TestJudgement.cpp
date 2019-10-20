#include "TestJudgement.h"

#include <cassert>

#include "Buff.h"
#include "CharacterStats.h"
#include "Equipment.h"
#include "Equipment.h"
#include "Item.h"
#include "Judgement.h"
#include "Paladin.h"
#include "PaladinSpells.h"
#include "Retribution.h"
#include "SanctityAura.h"
#include "SealOfCommand.h"
#include "SealOfTheCrusader.h"
#include "Talent.h"
#include "Vengeance.h"

TestJudgement::TestJudgement(EquipmentDb *equipment_db) :
    TestSpellPaladin(equipment_db, "Judgement")
{}

void TestJudgement::test_all() {
    run_mandatory_tests();

    set_up();
    test_resource_cost_1_of_5_benediction();
    tear_down();

    set_up();
    test_resource_cost_2_of_5_benediction();
    tear_down();

    set_up();
    test_resource_cost_3_of_5_benediction();
    tear_down();

    set_up();
    test_resource_cost_4_of_5_benediction();
    tear_down();

    set_up();
    test_resource_cost_5_of_5_benediction();
    tear_down();

    set_up();
    test_cooldown_1_of_2_improved_judgement();
    tear_down();

    set_up();
    test_cooldown_2_of_2_improved_judgement();
    tear_down();

    set_up();
    test_auto_hit_refreshes_judgement_of_the_crusader();
    tear_down();

    set_up();
    test_judgement_of_the_crusader_deals_zero_damage();
    tear_down();

    set_up();
    test_judgement_of_the_crusader_holy_dmg_bonus_0_of_3_improved_sotc();
    tear_down();

    set_up();
    test_judgement_of_the_crusader_holy_dmg_bonus_1_of_3_improved_sotc();
    tear_down();

    set_up();
    test_judgement_of_the_crusader_holy_dmg_bonus_2_of_3_improved_sotc();
    tear_down();

    set_up();
    test_judgement_of_the_crusader_holy_dmg_bonus_3_of_3_improved_sotc();
    tear_down();

    set_up();
    test_judgement_of_the_crusader_r10_pvp_glove_bonus_increases_holy_damage_bonus();
    tear_down();

    set_up();
    test_judgement_of_the_crusader_r13_pvp_glove_bonus_increases_holy_damage_bonus();
    tear_down();

    set_up(false);
    test_judgement_of_command_damage();
    tear_down();

    set_up(false);
    test_judgement_of_command_damage_with_sotc_and_sanctity_aura_and_vengeance();
    tear_down();
}

Judgement* TestJudgement::judgement() const {
    return dynamic_cast<Judgement*>(get_max_rank_spell_by_name("Judgement"));
}

void TestJudgement::test_name_correct() {
    assert(judgement()->get_name() == "Judgement");
}

void TestJudgement::test_spell_cooldown() {
    assert(almost_equal(judgement()->get_base_cooldown(), 10.0));
}

void TestJudgement::test_how_spell_observes_global_cooldown() {
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);

    assert(judgement()->get_spell_status() == SpellStatus::Available);

    given_paladin_is_on_gcd();

    assert(judgement()->get_spell_status() == SpellStatus::OnGCD);
}

void TestJudgement::test_resource_cost() {
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);

    given_paladin_has_mana(85);
    assert(judgement()->get_spell_status() == SpellStatus::InsufficientResources);

    given_paladin_has_mana(86);
    assert(judgement()->get_spell_status() == SpellStatus::Available);

    given_paladin_has_mana(87);
    when_judgement_is_performed();
    then_paladin_has_mana(1);
}

void TestJudgement::test_is_ready_conditions() {
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    given_paladin_has_mana(86);
    assert(judgement()->get_spell_status() == SpellStatus::Available);

    seal_of_the_crusader()->get_buff()->cancel_buff();

    assert(judgement()->get_spell_status() == SpellStatus::BuffInactive);
}

void TestJudgement::test_whether_spell_causes_global_cooldown() {
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    when_judgement_is_performed();

    assert(!paladin->action_ready());
}

void TestJudgement::test_resource_cost_1_of_5_benediction() {
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());
    given_retribution_talent_rank("Benediction", 1);

    given_paladin_has_mana(83);
    assert(judgement()->get_spell_status() == SpellStatus::InsufficientResources);

    given_paladin_has_mana(84);
    assert(judgement()->get_spell_status() == SpellStatus::Available);

    given_paladin_has_mana(85);
    when_judgement_is_performed();
    then_paladin_has_mana(1);
}

void TestJudgement::test_resource_cost_2_of_5_benediction() {
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());
    given_retribution_talent_rank("Benediction", 2);

    given_paladin_has_mana(80);
    assert(judgement()->get_spell_status() == SpellStatus::InsufficientResources);

    given_paladin_has_mana(81);
    assert(judgement()->get_spell_status() == SpellStatus::Available);

    given_paladin_has_mana(82);
    when_judgement_is_performed();
    then_paladin_has_mana(1);
}

void TestJudgement::test_resource_cost_3_of_5_benediction() {
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());
    given_retribution_talent_rank("Benediction", 3);

    given_paladin_has_mana(77);
    assert(judgement()->get_spell_status() == SpellStatus::InsufficientResources);

    given_paladin_has_mana(78);
    assert(judgement()->get_spell_status() == SpellStatus::Available);

    given_paladin_has_mana(79);
    when_judgement_is_performed();
    then_paladin_has_mana(1);
}

void TestJudgement::test_resource_cost_4_of_5_benediction() {
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());
    given_retribution_talent_rank("Benediction", 4);

    given_paladin_has_mana(75);
    assert(judgement()->get_spell_status() == SpellStatus::InsufficientResources);

    given_paladin_has_mana(76);
    assert(judgement()->get_spell_status() == SpellStatus::Available);

    given_paladin_has_mana(77);
    when_judgement_is_performed();
    then_paladin_has_mana(1);
}

void TestJudgement::test_resource_cost_5_of_5_benediction() {
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());
    given_retribution_talent_rank("Benediction", 5);

    given_paladin_has_mana(72);
    assert(judgement()->get_spell_status() == SpellStatus::InsufficientResources);

    given_paladin_has_mana(73);
    assert(judgement()->get_spell_status() == SpellStatus::Available);

    given_paladin_has_mana(74);
    when_judgement_is_performed();
    then_paladin_has_mana(1);
}

void TestJudgement::test_cooldown_1_of_2_improved_judgement() {
    given_retribution_talent_rank("Improved Judgement", 1);
    assert(almost_equal(judgement()->get_base_cooldown(), 9.0));
}

void TestJudgement::test_cooldown_2_of_2_improved_judgement() {
    given_retribution_talent_rank("Improved Judgement", 2);
    assert(almost_equal(judgement()->get_base_cooldown(), 8.0));
}

void TestJudgement::test_auto_hit_refreshes_judgement_of_the_crusader() {
    given_a_mainhand_weapon_with_3_speed();
    given_a_guaranteed_white_hit();
    given_a_guaranteed_ranged_white_hit();
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());
    when_judgement_is_performed();

    given_engine_priority_pushed_forward(5.0);
    assert(almost_equal(seal_of_the_crusader()->get_judge_debuff()->time_left(), 5.0));
    when_mh_attack_is_performed();

    assert(almost_equal(seal_of_the_crusader()->get_judge_debuff()->time_left(), 10.0));
}

void TestJudgement::test_judgement_of_the_crusader_deals_zero_damage() {
    given_a_guaranteed_ranged_white_hit();
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    when_judgement_is_performed();

    then_damage_dealt_is(0);
}

void TestJudgement::test_judgement_of_the_crusader_holy_dmg_bonus_0_of_3_improved_sotc() {
    given_a_guaranteed_ranged_white_hit();
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 0);
    when_judgement_is_performed();

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 140);
}

void TestJudgement::test_judgement_of_the_crusader_holy_dmg_bonus_1_of_3_improved_sotc() {
    given_a_guaranteed_ranged_white_hit();
    given_retribution_talent_rank("Improved Seal of the Crusader", 1);
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 0);
    when_judgement_is_performed();

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 147);
}

void TestJudgement::test_judgement_of_the_crusader_holy_dmg_bonus_2_of_3_improved_sotc() {
    given_a_guaranteed_ranged_white_hit();
    given_retribution_talent_rank("Improved Seal of the Crusader", 2);
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 0);
    when_judgement_is_performed();

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 154);
}

void TestJudgement::test_judgement_of_the_crusader_holy_dmg_bonus_3_of_3_improved_sotc() {
    given_a_guaranteed_ranged_white_hit();
    given_retribution_talent_rank("Improved Seal of the Crusader", 3);
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 0);
    when_judgement_is_performed();

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 161);
}

void TestJudgement::test_judgement_of_the_crusader_r10_pvp_glove_bonus_increases_holy_damage_bonus() {
    given_gloves_equipped(23274);
    assert(pchar->get_stats()->get_equipment()->get_gloves()->name == "Knight-Lieutenant's Lamellar Gauntlets");
    given_a_guaranteed_ranged_white_hit();
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 0);
    when_judgement_is_performed();

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 140 + 20);
}

void TestJudgement::test_judgement_of_the_crusader_r13_pvp_glove_bonus_increases_holy_damage_bonus() {
    given_gloves_equipped(16471);
    assert(pchar->get_stats()->get_equipment()->get_gloves()->name == "Marshal's Lamellar Gloves");
    given_a_guaranteed_ranged_white_hit();
    when_seal_of_the_crusader_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 0);
    when_judgement_is_performed();

    assert(paladin->get_stats()->get_spell_damage(MagicSchool::Holy) == 140 + 20);
}

void TestJudgement::test_judgement_of_command_damage() {
    given_seal_of_command_is_enabled();
    given_a_guaranteed_ranged_white_hit();
    when_seal_of_command_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());

    when_judgement_is_performed();

    // [Damage] = r1_joc_min <= dmg <= r1_joc_max
    // [47-56] = 47 <= dmg <= 56
    then_damage_dealt_is_in_range(47, 56);
}

void TestJudgement::test_judgement_of_command_damage_with_sotc_and_sanctity_aura_and_vengeance() {
    given_sanctity_aura_is_active();
    given_vengeance_is_active(5);
    given_seal_of_command_is_enabled();
    given_a_guaranteed_ranged_white_hit();
    paladin->reset();
    when_seal_of_command_is_performed();
    given_engine_priority_pushed_forward(1.5);
    assert(paladin->action_ready());
    given_character_has_spell_damage(100, MagicSchool::Holy);

    when_judgement_is_performed();

    // [Damage] = ((r1_joc_min + holy_spell_dmg * 0.43) * spell_dmg_mod) <= dmg <= ((r1_joc_max + holy_spell_dmg * 0.43) * spell_dmg_mod)
    // [114-125] = ((47 + 100 * 0.43) * 1.32) <= dmg <= ((56 + 100 * 0.43) * 1.32)
    then_damage_dealt_is_in_range(114, 125);
}

void TestJudgement::when_judgement_is_performed() {
    assert(judgement()->get_spell_status() == SpellStatus::Available);
    judgement()->perform();
}

void TestJudgement::given_sanctity_aura_is_active() {
    given_retribution_talent_rank("Sanctity Aura", 1);
    get_max_rank_spell_by_name("Sanctity Aura")->perform();
    given_engine_priority_pushed_forward(1.5);
}

void TestJudgement::given_vengeance_is_active(const unsigned num) {
    auto ret = Retribution(paladin);
    given_talent_rank(ret, "Conviction", 5);
    given_talent_rank(ret, "Vengeance", num);
    paladin->get_vengeance()->apply_buff();
}

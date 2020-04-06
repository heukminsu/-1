#include "TestSpellDruid.h"

#include <cassert>
#include <utility>

#include "Balance.h"
#include "BearForm.h"
#include "Buff.h"
#include "CatForm.h"
#include "CharacterStats.h"
#include "Druid.h"
#include "DruidSpells.h"
#include "Equipment.h"
#include "FeralCombat.h"
#include "FerociousBite.h"
#include "Item.h"
#include "Maul.h"
#include "Moonfire.h"
#include "MoonkinForm.h"
#include "RaidControl.h"
#include "RestorationDruid.h"
#include "Shred.h"
#include "SimSettings.h"
#include "Spell.h"
#include "Starfire.h"
#include "Swipe.h"
#include "Talent.h"
#include "Wrath.h"

TestSpellDruid::TestSpellDruid(EquipmentDb* equipment_db, QString spell_under_test) :
    TestSpellDamage(equipment_db, std::move(spell_under_test)), druid(nullptr) {}

void TestSpellDruid::set_up(const bool prepare_combat_iterations) {
    set_up_general();
    druid = new Druid(race, equipment_db, sim_settings, raid_control);
    druid->set_clvl(60);
    pchar = druid;

    if (prepare_combat_iterations) {
        raid_control->prepare_set_of_combat_iterations();
        pchar->prepare_set_of_combat_iterations();
    }
}

void TestSpellDruid::tear_down() {
    delete druid;
    tear_down_general();
}

void TestSpellDruid::run_class_specific_tests() {}

Wrath* TestSpellDruid::wrath() const {
    return static_cast<Wrath*>(get_max_rank_spell_by_name("Wrath"));
}

Moonfire* TestSpellDruid::moonfire() const {
    return static_cast<Moonfire*>(get_max_rank_spell_by_name("Moonfire"));
}

Starfire* TestSpellDruid::starfire() const {
    return static_cast<Starfire*>(get_max_rank_spell_by_name("Starfire"));
}

MoonkinForm* TestSpellDruid::moonkin_form() const {
    return static_cast<MoonkinForm*>(get_max_rank_spell_by_name("Moonkin Form"));
}

CatForm* TestSpellDruid::cat_form() const {
    return static_cast<CatForm*>(get_max_rank_spell_by_name("Cat Form"));
}

BearForm* TestSpellDruid::bear_form() const {
    return static_cast<BearForm*>(get_max_rank_spell_by_name("Bear Form"));
}

Shred* TestSpellDruid::shred() const {
    return static_cast<Shred*>(get_max_rank_spell_by_name("Shred"));
}

Swipe* TestSpellDruid::swipe() const {
    return static_cast<Swipe*>(get_max_rank_spell_by_name("Swipe"));
}

Maul* TestSpellDruid::maul() const {
    return static_cast<Maul*>(get_max_rank_spell_by_name("Maul"));
}

FerociousBite* TestSpellDruid::ferocious_bite() const {
    return static_cast<FerociousBite*>(get_max_rank_spell_by_name("Ferocious Bite"));
}

void TestSpellDruid::given_balance_talent_rank(const QString& talent_name, const unsigned num) {
    given_talent_rank(Balance(druid), talent_name, num);
}

void TestSpellDruid::given_feral_talent_rank(const QString& talent_name, const unsigned num) {
    given_talent_rank(FeralCombat(druid), talent_name, num);
}

void TestSpellDruid::given_restoration_talent_rank(const QString& talent_name, const unsigned num) {
    given_talent_rank(RestorationDruid(druid), talent_name, num);
}

void TestSpellDruid::given_balance_talent_ranks(const QVector<QPair<QString, unsigned>>& talent_ranks) {
    given_talent_ranks(Balance(druid), talent_ranks);
}

void TestSpellDruid::given_feral_talent_ranks(const QVector<QPair<QString, unsigned>>& talent_ranks) {
    given_talent_ranks(FeralCombat(druid), talent_ranks);
}

void TestSpellDruid::given_druid_has_combo_points(const unsigned num) {
    druid->spend_combo_points();

    if (num == 0) {
        assert(druid->get_combo_points() == 0);
        return;
    }

    druid->gain_combo_points(num);
    assert(druid->get_combo_points() == num);
}

void TestSpellDruid::given_druid_is_on_gcd(Spell* spell) {
    unsigned mana_before = druid->get_resource_level(ResourceType::Mana);
    druid->gain_mana(static_cast<unsigned>(spell->get_resource_cost()));

    spell->perform();

    int mana_delta = static_cast<int>(druid->get_resource_level(ResourceType::Mana)) - static_cast<int>(mana_before);

    if (mana_delta < 0)
        druid->gain_mana(static_cast<unsigned>(mana_delta * -1));
    else
        druid->lose_mana(static_cast<unsigned>(mana_delta));

    assert(druid->on_global_cooldown());
}

void TestSpellDruid::given_druid_has_mana(const unsigned mana) {
    if (druid->get_resource_level(ResourceType::Mana) > 0)
        druid->lose_mana(druid->get_resource_level(ResourceType::Mana));
    druid->gain_mana(mana);
    then_druid_has_mana(mana);
}

void TestSpellDruid::given_druid_has_energy(const unsigned energy) {
    if (druid->get_resource_level(ResourceType::Energy) > 0)
        druid->lose_energy(druid->get_resource_level(ResourceType::Energy));
    druid->gain_energy(energy);
    then_druid_has_energy(energy);
}

void TestSpellDruid::given_druid_has_rage(const unsigned rage) {
    if (druid->get_resource_level(ResourceType::Rage) > 0)
        druid->lose_energy(druid->get_resource_level(ResourceType::Rage));
    druid->gain_rage(rage);
    then_druid_has_rage(rage);
}

void TestSpellDruid::then_druid_has_mana(const unsigned mana) {
    if (mana != druid->get_resource_level(ResourceType::Mana))
        qDebug() << "Expected" << mana << "mana but has" << druid->get_resource_level(ResourceType::Mana);
    assert(druid->get_resource_level(ResourceType::Mana) == mana);
}

void TestSpellDruid::then_druid_has_energy(const unsigned energy) {
    if (energy != druid->get_resource_level(ResourceType::Energy))
        qDebug() << "Expected" << energy << "energy but has" << druid->get_resource_level(ResourceType::Energy);
    assert(druid->get_resource_level(ResourceType::Energy) == energy);
}

void TestSpellDruid::then_druid_has_rage(const unsigned rage) {
    if (rage != druid->get_resource_level(ResourceType::Rage))
        qDebug() << "Expected" << rage << "rage but has" << druid->get_resource_level(ResourceType::Rage);
    assert(druid->get_resource_level(ResourceType::Rage) == rage);
}

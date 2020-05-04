#include "TestMage.h"

#include "CharacterStats.h"
#include "Equipment.h"
#include "Item.h"
#include "ItemNamespace.h"
#include "Mage.h"
#include "RaidControl.h"
#include "SimSettings.h"
#include "TestArcane.h"
#include "TestArcaneMissiles.h"
#include "TestEvocation.h"
#include "TestFire.h"
#include "TestFireball.h"
#include "TestFrost.h"
#include "TestFrostbolt.h"
#include "TestMageArmor.h"
#include "TestMageTalentStatIncrease.h"
#include "TestScorch.h"
#include "Troll.h"

TestMage::TestMage(EquipmentDb* equipment_db) : TestObject(equipment_db) {}

void TestMage::test_all() {
    test_values_after_initialization();

    TestArcane(equipment_db).test_all();
    TestFire(equipment_db).test_all();
    TestFrost(equipment_db).test_all();

    TestMageTalentStatIncrease(equipment_db).test_all();
    TestFireball(equipment_db).test_all();
    TestScorch(equipment_db).test_all();
    TestEvocation(equipment_db).test_all();
    TestMageArmor(equipment_db).test_all();
    TestFrostbolt(equipment_db).test_all();
    TestArcaneMissiles(equipment_db).test_all();
}

void TestMage::test_values_after_initialization() {
    Race* race = new Troll();
    auto sim_settings = new SimSettings();
    auto raid_control = new RaidControl(sim_settings);
    auto mage = new Mage(race, equipment_db, sim_settings, raid_control);

    assert(mage->class_name == "Mage");
    assert(mage->get_race()->get_name() == "Troll");
    assert(mage->get_highest_possible_armor_type() == ArmorTypes::CLOTH);
    assert(almost_equal(1.5, mage->global_cooldown()));
    assert(almost_equal(2.0, mage->get_stats()->get_melee_ability_crit_dmg_mod()));
    assert(almost_equal(1.5, mage->get_stats()->get_spell_crit_dmg_mod()));

    mage->get_equipment()->set_wrist(22503);
    assert(mage->get_equipment()->get_wrist()->name == "Frostfire Bindings");
    mage->get_equipment()->clear_wrist();

    delete race;
    delete mage;
    delete sim_settings;
    delete raid_control;
}

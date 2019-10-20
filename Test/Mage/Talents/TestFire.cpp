#include "TestFire.h"

#include <cassert>

#include "Mage.h"
#include "RaidControl.h"
#include "SimSettings.h"
#include "Troll.h"

TestFire::TestFire(EquipmentDb* equipment_db):
    TestTalentTree(equipment_db, "MID")
{}

void TestFire::set_up() {
    race = new Troll();
    sim_settings = new SimSettings();
    raid_control = new RaidControl(sim_settings);
    pchar = new Mage(race, equipment_db, sim_settings, raid_control);
}

void TestFire::tear_down() {
    delete pchar;
    delete race;
    delete sim_settings;
    delete raid_control;
}

void TestFire::test_spending_talent_points() {
    assert(increment("Improved Fireball", 5));
    assert(increment("Impact", 5));
    assert(increment("Ignite", 5));
    assert(increment("Flame Throwing", 2));
    assert(increment("Improved Fire Blast", 3));

    // Assert cannot spend points into Blast Wave if 1/1 Pyroblast is missing
    assert(tree_has_points(20));

    assert(!increment("Blast Wave"));
    assert(increment("Pyroblast"));
    assert(increment("Blast Wave"));

    // Assert cannot remove points from Pyroblast once points are spent in Blast Wave
    assert(!decrement("Pyroblast"));
    assert(decrement("Blast Wave"));
    assert(decrement("Pyroblast"));

    // Assert cannot increment Combustion without Blast Wave
    assert(increment("Incinerate", 2));
    assert(increment("Improved Flamestrike", 3));
    assert(increment("Fire Power", 5));
    assert(tree_has_points(30));

    assert(!increment("Combustion"));
    assert(increment("Critical Mass"));
    assert(!increment("Combustion"));
    assert(increment("Critical Mass"));
    assert(!increment("Combustion"));
    assert(increment("Critical Mass"));
    assert(increment("Combustion"));
    assert(!decrement("Critical Mass"));
}

void TestFire::test_clearing_tree_after_filling() {
    spec_fire();

    clear_tree();

    assert(tree_has_points(0));
}

void TestFire::test_refilling_tree_after_switching_talent_setup() {
    assert(tree_has_points(0));
    spec_fire();
    assert(tree_has_points(46));

    switch_to_setup(1);
    assert(tree_has_points(0));
    spec_fire();
    assert(tree_has_points(46));

    switch_to_setup(2);
    assert(tree_has_points(0));
    spec_fire();
    assert(tree_has_points(46));
}

void TestFire::spec_fire() {
    assert(increment("Improved Fireball", 5));
    assert(increment("Impact", 5));
    assert(increment("Ignite", 5));
    assert(increment("Flame Throwing", 2));
    assert(increment("Improved Fire Blast", 3));
    assert(increment("Incinerate", 2));
    assert(increment("Improved Flamestrike", 3));
    assert(increment("Pyroblast", 1));
    assert(increment("Burning Soul", 2));
    assert(increment("Improved Scorch", 3));
    assert(increment("Improved Fire Ward", 2));
    assert(increment("Master of Elements", 3));
    assert(increment("Critical Mass", 3));
    assert(increment("Blast Wave", 1));
    assert(increment("Fire Power", 5));
    assert(increment("Combustion", 1));
}

#pragma once

#include "TestSpellMage.h"

class TestMageArmor : public TestSpellMage {
public:
    TestMageArmor(EquipmentDb* equipment_db);

    void test_all();

private:
    void test_name_correct() override;
    void test_spell_cooldown() override;
    void test_whether_spell_causes_global_cooldown() override;
    void test_how_spell_observes_global_cooldown() override;
    void test_resource_cost() override;
    void test_is_ready_conditions() override;

    void test_mana_regen_increase();

    void when_mage_armor_is_performed();
};

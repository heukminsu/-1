#include "MageArmorBuff.h"

#include "Spells/Buff.h"
#include "Mage.h"
#include "Mana.h"
#include "Utils/Check.h"

MageArmorBuff::MageArmorBuff(Mage* mage) :
    SelfBuff(mage, "Mage Armor", "Assets/spell/Spell_arcane_mage_armor.png", BuffDuration::PERMANENT, 0), mana(static_cast<Mana*>(mage->get_resource())) {
    check((mana != nullptr), "MageArmorBuff:: Mana nullptr");
}

void MageArmorBuff::buff_effect_when_applied() {
    mana->mp5_from_spirit_within_5sr_modifier += 0.3;
}

void MageArmorBuff::buff_effect_when_removed() {
    mana->mp5_from_spirit_within_5sr_modifier -= 0.3;
}

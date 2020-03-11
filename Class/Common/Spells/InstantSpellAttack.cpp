#include "InstantSpellAttack.h"

#include "Character.h"
#include "CharacterStats.h"
#include "CombatRoll.h"
#include "CooldownControl.h"
#include "Random.h"

InstantSpellAttack::InstantSpellAttack(Character* pchar,
                                       const QString& name,
                                       const QString& icon,
                                       const MagicSchool school,
                                       const unsigned min,
                                       const unsigned max,
                                       const double coefficient,
                                       const ConsumeCharge consume_charge,
                                       const unsigned innate_threat) :
    Spell(name, icon, pchar, new CooldownControl(pchar, 0.0), RestrictedByGcd::No, ResourceType::Rage, 0),
    school(school),
    min(min),
    max(max),
    coefficient(coefficient),
    consume_charge(consume_charge),
    innate_threat(innate_threat),
    random(new Random(min, max)) {}

InstantSpellAttack::~InstantSpellAttack() {
    delete random;
    delete cooldown;
}

void InstantSpellAttack::spell_effect() {
    const int hit_roll = roll->get_spell_ability_result(school, pchar->get_stats()->get_spell_crit_chance(school));
    const int resist_roll = roll->get_spell_resist_result(school);

    if (hit_roll == MagicAttackResult::MISS)
        return increment_miss();
    if (resist_roll == MagicResistResult::FULL_RESIST)
        return increment_full_resist();

    unsigned damage_dealt = random->get_roll();
    damage_dealt += static_cast<unsigned>(round(pchar->get_stats()->get_spell_damage(school, consume_charge) * coefficient));
    const double resist_mod = get_partial_resist_dmg_modifier(resist_roll);
    const double damage_mod = pchar->get_stats()->get_magic_school_damage_mod(school, consume_charge);

    if (hit_roll == MagicAttackResult::CRITICAL) {
        pchar->spell_critical_effect(school);
        add_spell_crit_dmg(static_cast<int>(round(damage_dealt * damage_mod * pchar->get_stats()->get_spell_crit_dmg_mod() * resist_mod)),
                           get_resource_cost(), 0, resist_roll, innate_threat);
    } else {
        pchar->spell_hit_effect(school);
        add_spell_hit_dmg(static_cast<int>(round(damage_dealt * damage_mod * resist_mod)), get_resource_cost(), 0, resist_roll, innate_threat);
    }
}

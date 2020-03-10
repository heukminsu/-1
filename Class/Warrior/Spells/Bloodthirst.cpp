#include "Bloodthirst.h"

#include "Buff.h"
#include "CharacterStats.h"
#include "CombatRoll.h"
#include "CooldownControl.h"
#include "Warrior.h"
#include "WarriorSpells.h"

Bloodthirst::Bloodthirst(Warrior* pchar, WarriorSpells* spells, CooldownControl* cooldown_control) :
    Spell("Bloodthirst", "Assets/spell/Spell_nature_bloodlust.png", pchar, cooldown_control, RestrictedByGcd::Yes, ResourceType::Rage, 30),
    TalentRequirer(QVector<TalentRequirerInfo*> {new TalentRequirerInfo("Bloodthirst", 1, DisabledAtZero::Yes)}),
    warr(pchar),
    spells(spells) {
    this->enabled = false;
}

void Bloodthirst::spell_effect() {
    const int result = roll->get_melee_ability_result(warr->get_mh_wpn_skill(), pchar->get_stats()->get_mh_crit_chance());

    cooldown->add_spell_cd_event();
    cooldown->add_gcd_event();

    if (result == PhysicalAttackResult::MISS) {
        increment_miss();
        warr->lose_rage(resource_cost);
        return;
    }
    if (result == PhysicalAttackResult::DODGE) {
        increment_dodge();
        spells->get_overpower_buff()->apply_buff();
        warr->lose_rage(static_cast<unsigned>(round(resource_cost * 0.25)));
        return;
    }
    if (result == PhysicalAttackResult::PARRY) {
        increment_parry();
        warr->lose_rage(static_cast<unsigned>(round(resource_cost * 0.25)));
        return;
    }

    warr->lose_rage(resource_cost);

    double damage_dealt = damage_after_modifiers(warr->get_stats()->get_melee_ap() * 0.45);

    if (result == PhysicalAttackResult::CRITICAL) {
        damage_dealt *= warr->get_stats()->get_melee_ability_crit_dmg_mod();
        warr->melee_mh_yellow_critical_effect();
        add_crit_dmg(static_cast<int>(round(damage_dealt)), resource_cost, pchar->global_cooldown());
    } else if (result == PhysicalAttackResult::HIT) {
        warr->melee_mh_yellow_hit_effect();
        add_hit_dmg(static_cast<int>(round(damage_dealt)), resource_cost, pchar->global_cooldown());
    }
}

SpellStatus Bloodthirst::is_ready_spell_specific() const {
    return warr->on_stance_cooldown() ? SpellStatus::OnStanceCooldown : SpellStatus::Available;
}

void Bloodthirst::increase_talent_rank_effect(const QString&, const int) {}

void Bloodthirst::decrease_talent_rank_effect(const QString&, const int) {}

#include "Spell.h"

#include <utility>

#include "Character.h"
#include "CharacterSpells.h"
#include "CharacterStats.h"
#include "ClassStatistics.h"
#include "CombatRoll.h"
#include "CooldownControl.h"
#include "Engine.h"
#include "Mechanics.h"
#include "RaidControl.h"
#include "StatisticsSpell.h"
#include "Target.h"
#include "Utils/Check.h"

Spell::Spell(QString name,
             QString icon,
             Character* pchar,
             CooldownControl* cooldown_control,
             const RestrictedByGcd restricted_by_gcd,
             const ResourceType resource_type,
             const unsigned resource_cost,
             const int spell_rank) :
    name(std::move(name)),
    icon(std::move(icon)),
    pchar(pchar),
    engine(pchar->get_engine()),
    roll(pchar->get_combat_roll()),
    cooldown(cooldown_control),
    statistics_spell(nullptr),
    restricted_by_gcd(restricted_by_gcd),
    resource_type(resource_type),
    resource_cost(resource_cost),
    spell_rank(spell_rank),
    instance_id(InstanceID::INACTIVE),
    enabled(true) {}

bool operator==(const Spell& lhs, const Spell& rhs) {
    return lhs.get_instance_id() == rhs.get_instance_id();
}

QString Spell::get_name() const {
    return this->name;
}

QString Spell::get_icon() const {
    return this->icon;
}

double Spell::get_base_cooldown() const {
    return this->cooldown->base;
}

double Spell::get_last_used() const {
    return this->cooldown->last_used;
}

double Spell::get_next_use() const {
    return cooldown->get_next_use();
}

double Spell::get_resource_cost() const {
    const unsigned mana_skill_reduction = resource_type == ResourceType::Mana ? pchar->get_stats()->get_mana_skill_reduction() : 0;
    const unsigned calculated_resource_cost = static_cast<unsigned>(round(resource_cost * resource_cost_mod));
    return mana_skill_reduction > calculated_resource_cost ? 0 : calculated_resource_cost - mana_skill_reduction;
}

SpellStatus Spell::is_ready_spell_specific() const {
    return SpellStatus::Available;
}

void Spell::enable_spell_effect() {}

void Spell::disable_spell_effect() {}

SpellStatus Spell::get_spell_status() const {
    if (!enabled)
        return SpellStatus::NotEnabled;

    if (restricted_by_gcd == RestrictedByGcd::Yes && pchar->on_global_cooldown())
        return SpellStatus::OnGCD;

    if (pchar->get_spells()->cast_in_progress())
        return SpellStatus::CastInProgress;

    if ((get_next_use() - engine->get_current_priority()) > 0.0001)
        return SpellStatus::OnCooldown;

    if (pchar->get_resource_level(resource_type) < get_resource_cost())
        return SpellStatus::InsufficientResources;

    return is_ready_spell_specific();
}

bool Spell::is_enabled() const {
    return enabled;
}

bool Spell::is_rank_learned() const {
    return true;
}

int Spell::get_spell_rank() const {
    return spell_rank;
}

void Spell::enable() {
    check(!enabled, QString("Tried to enable an already enabled spell '%1'").arg(name).toStdString());
    enabled = true;
    enable_spell_effect();
}

void Spell::disable() {
    if (enabled)
        disable_spell_effect();

    enabled = false;
}

double Spell::get_cooldown_remaining() const {
    return cooldown->get_cooldown_remaining();
}

void Spell::perform() {
    check((pchar->get_resource_level(resource_type) >= get_resource_cost()),
          QString("Tried to perform '%1' but has unsufficient resource %2 (requires %3)")
              .arg(name)
              .arg(pchar->get_resource_level(resource_type))
              .arg(get_resource_cost())
              .toStdString());
    cooldown->last_used = engine->get_current_priority();
    this->spell_effect();
}

void Spell::increment_miss() {
    statistics_spell->increment_miss();
}

void Spell::increment_full_resist() {
    statistics_spell->increment_full_resist();
}

void Spell::increment_dodge() {
    statistics_spell->increment_dodge();
}

void Spell::increment_parry() {
    statistics_spell->increment_parry();
}

void Spell::increment_full_block() {
    statistics_spell->increment_full_block();
}

void Spell::add_partial_block_dmg(const int damage, const double resource_cost, const double execution_time, const int innate_threat) {
    statistics_spell->add_partial_block_dmg(damage, resource_cost, execution_time);
    auto threat = (damage + innate_threat) * pchar->get_stats()->get_total_threat_mod() ;
    statistics_spell->add_partial_block_thrt(threat, resource_cost, execution_time);
}

void Spell::add_partial_block_crit_dmg(const int damage, const double resource_cost, const double execution_time, const int innate_threat) {
    statistics_spell->add_partial_block_crit_dmg(damage, resource_cost, execution_time);
    auto threat = (damage + innate_threat) * pchar->get_stats()->get_total_threat_mod() ;
    statistics_spell->add_partial_block_crit_thrt(threat, resource_cost, execution_time);
}

void Spell::add_glancing_dmg(const int damage, const double resource_cost, const double execution_time, const int innate_threat) {
    statistics_spell->add_glancing_dmg(damage, resource_cost, execution_time);
    auto threat = (damage + innate_threat) * pchar->get_stats()->get_total_threat_mod() ;
    statistics_spell->add_glancing_thrt(threat, resource_cost, execution_time);
}

void Spell::add_hit_dmg(const int damage, const double resource_cost, const double execution_time, const int innate_threat) {
    statistics_spell->add_hit_dmg(damage, resource_cost, execution_time);
    auto threat = (damage + innate_threat) * pchar->get_stats()->get_total_threat_mod() ;
    statistics_spell->add_hit_thrt(threat, resource_cost, execution_time);
}

void Spell::add_spell_hit_dmg(const int damage, const double resource_cost, const double execution_time, const int resist_result, const int innate_threat) {
    statistics_spell->add_spell_hit_dmg(damage, resource_cost, execution_time, resist_result);
    auto threat = (damage + innate_threat) * pchar->get_stats()->get_total_threat_mod() ;
    statistics_spell->add_spell_hit_thrt(threat, resource_cost, execution_time, resist_result);
}

void Spell::add_crit_dmg(const int damage, const double resource_cost, const double execution_time, const int innate_threat) {
    statistics_spell->add_crit_dmg(damage, resource_cost, execution_time);
    auto threat = (damage + innate_threat) * pchar->get_stats()->get_total_threat_mod() ;
    statistics_spell->add_crit_thrt(threat, resource_cost, execution_time);
}

void Spell::add_spell_crit_dmg(const int damage, const double resource_cost, const double execution_time, const int resist_result, const int innate_threat) {
    statistics_spell->add_spell_crit_dmg(damage, resource_cost, execution_time, resist_result);
    auto threat = (damage + innate_threat) * pchar->get_stats()->get_total_threat_mod() ;
    statistics_spell->add_spell_crit_thrt(threat, resource_cost, execution_time, resist_result);
}

double Spell::damage_after_modifiers(const double damage) const {
    const double armor_reduction = 1 - Mechanics::get_reduction_from_armor(pchar->get_target()->get_armor(), pchar->get_clvl());
    return (damage * pchar->get_stats()->get_total_physical_damage_mod() + pchar->get_stats()->get_flat_physical_damage_bonus()) * armor_reduction;
}

double Spell::get_partial_resist_dmg_modifier(const int resist_result) const {
    switch (resist_result) {
    case MagicResistResult::FULL_RESIST:
        return 0.0;
    case MagicResistResult::PARTIAL_RESIST_75:
        return 0.25;
    case MagicResistResult::PARTIAL_RESIST_50:
        return 0.5;
    case MagicResistResult::PARTIAL_RESIST_25:
        return 0.75;
    case MagicResistResult::NO_RESIST:
        return 1.0;
    default:
        check(false, "Spell::get_partial_resist_dmg_modifier reached end of switch");
        return 0.0;
    }
}

void Spell::reset() {
    cooldown->reset();
    reset_effect();
}

void Spell::prepare_set_of_combat_iterations() {
    prepare_set_of_combat_iterations_spell_specific();
    this->statistics_spell = pchar->get_statistics()->get_spell_statistics(name, icon, spell_rank);
}

int Spell::get_instance_id() const {
    return this->instance_id;
}

void Spell::set_instance_id(const int instance_id) {
    this->instance_id = instance_id;
}

void Spell::increase_resource_cost_modifier(const int change) {
    CharacterStats::add_multiplicative_effect(resource_cost_mod_changes, change, resource_cost_mod);
}

void Spell::decrease_resource_cost_modifier(const int change) {
    CharacterStats::remove_multiplicative_effect(resource_cost_mod_changes, change, resource_cost_mod);
}

void Spell::prepare_set_of_combat_iterations_spell_specific() {}

void Spell::reset_effect() {}

void Spell::perform_start_of_combat() {
    check(false, QString("%1 has not defined Spell::perform_start_of_combat").arg(name).toStdString());
}

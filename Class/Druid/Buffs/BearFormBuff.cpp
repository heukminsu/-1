#include "BearFormBuff.h"

#include <cmath>

#include "CharacterStats.h"
#include "Druid.h"
#include "Equipment.h"
#include "Proc.h"

BearFormBuff::BearFormBuff(Druid* pchar, Buff* leader_of_the_pack, Proc* furor) :
    SelfBuff(pchar, "Bear Form", "Assets/ability/Ability_racial_bearform.png", BuffDuration::PERMANENT, 0),
    TalentRequirer({
        new TalentRequirerInfo("Sharpened Claws", 3, DisabledAtZero::No),
        new TalentRequirerInfo("Predatory Strikes", 3, DisabledAtZero::No),
        new TalentRequirerInfo("Heart of the Wild", 5, DisabledAtZero::No),
        new TalentRequirerInfo("Leader of the Pack", 1, DisabledAtZero::No),
        new TalentRequirerInfo("Feral Instinct", 5, DisabledAtZero::No),
    }),
    druid(pchar),
    leader_of_the_pack(leader_of_the_pack),
    furor(furor),
    threat_mod(30) {}

void BearFormBuff::buff_effect_when_applied() {
    pchar->get_equipment()->druid_bear_form_switch_to_paws();

    if (sharpened_claws_bonus > 0)
        druid->get_stats()->increase_melee_aura_crit(sharpened_claws_bonus);

    if (predatory_strikes_bonus > 0)
        druid->get_stats()->increase_melee_ap(predatory_strikes_bonus);

    if (heart_of_the_wild_mod > 0)
        druid->get_stats()->add_strength_mod(heart_of_the_wild_mod);

    if (supplies_leader_of_the_pack)
        leader_of_the_pack->apply_buff();

    if (furor->is_enabled() && furor->check_proc_success())
        furor->perform();

    druid->get_stats()->increase_total_threat_mod(threat_mod);
}

void BearFormBuff::buff_effect_when_removed() {
    pchar->get_equipment()->druid_switch_to_normal_weapon();

    if (sharpened_claws_bonus > 0)
        druid->get_stats()->decrease_melee_aura_crit(sharpened_claws_bonus);

    if (predatory_strikes_bonus > 0)
        druid->get_stats()->decrease_melee_ap(predatory_strikes_bonus);

    if (heart_of_the_wild_mod > 0)
        druid->get_stats()->remove_strength_mod(heart_of_the_wild_mod);

    if (supplies_leader_of_the_pack)
        leader_of_the_pack->cancel_buff();

    druid->get_stats()->decrease_total_threat_mod(threat_mod);
}

void BearFormBuff::increase_talent_rank_effect(const QString& talent_name, const int curr) {
    if (talent_name == "Sharpened Claws") {
        if (is_active() && curr > 1)
            druid->get_stats()->decrease_melee_aura_crit(sharpened_claws_bonus);

        sharpened_claws_bonus = sharpened_claws_ranks[curr];

        if (is_active())
            druid->get_stats()->increase_melee_aura_crit(sharpened_claws_bonus);
    }

    if (talent_name == "Predatory Strikes") {
        if (is_active() && curr > 1)
            druid->get_stats()->decrease_melee_ap(predatory_strikes_bonus);

        predatory_strikes_bonus = static_cast<unsigned>(std::round(pchar->get_clvl() * predatory_strikes_ranks[curr]));

        if (is_active())
            druid->get_stats()->increase_melee_ap(predatory_strikes_bonus);
    }

    if (talent_name == "Heart of the Wild") {
        if (curr > 1) {
            druid->get_stats()->remove_intellect_mod(heart_of_the_wild_mod);
            if (is_active())
                druid->get_stats()->remove_strength_mod(heart_of_the_wild_mod);
        }

        heart_of_the_wild_mod = heart_of_the_wild_str_mod_ranks[curr];

        if (is_active())
            druid->get_stats()->add_strength_mod(heart_of_the_wild_mod);
        druid->get_stats()->add_intellect_mod(heart_of_the_wild_mod);
    }

    if (talent_name == "Leader of the Pack") {
        supplies_leader_of_the_pack = true;
        if (is_active())
            leader_of_the_pack->apply_buff();
    }

    if (talent_name == "Feral Instinct") {
        threat_mod = 30 + feral_instinct_ranks[curr];
    }
}

void BearFormBuff::decrease_talent_rank_effect(const QString& talent_name, const int curr) {
    if (talent_name == "Sharpened Claws") {
        if (is_active())
            druid->get_stats()->decrease_melee_aura_crit(sharpened_claws_bonus);

        sharpened_claws_bonus = sharpened_claws_ranks[curr];

        if (is_active() && sharpened_claws_bonus > 0)
            druid->get_stats()->increase_melee_aura_crit(sharpened_claws_bonus);
    }

    if (talent_name == "Predatory Strikes") {
        if (is_active())
            druid->get_stats()->decrease_melee_ap(predatory_strikes_bonus);

        predatory_strikes_bonus = static_cast<unsigned>(std::round(pchar->get_clvl() * predatory_strikes_ranks[curr]));

        if (is_active() && predatory_strikes_bonus > 0)
            druid->get_stats()->increase_melee_ap(predatory_strikes_bonus);
    }

    if (talent_name == "Heart of the Wild") {
        if (is_active())
            druid->get_stats()->remove_strength_mod(heart_of_the_wild_mod);
        druid->get_stats()->remove_intellect_mod(heart_of_the_wild_mod);

        heart_of_the_wild_mod = heart_of_the_wild_str_mod_ranks[curr];

        if (heart_of_the_wild_mod > 0) {
            druid->get_stats()->add_intellect_mod(heart_of_the_wild_mod);
            if (is_active())
                druid->get_stats()->add_strength_mod(heart_of_the_wild_mod);
        }
    }

    if (talent_name == "Leader of the Pack") {
        supplies_leader_of_the_pack = false;
        if (is_active())
            leader_of_the_pack->cancel_buff();
    }

    if (talent_name == "Feral Instinct") {
        threat_mod = 30 + feral_instinct_ranks[curr];
    }
}

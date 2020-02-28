#include "PaladinSpells.h"

#include "Buff.h"
#include "Consecration.h"
#include "Judgement.h"
#include "JudgementOfTheCrusader.h"
#include "MainhandAttackPaladin.h"
#include "Paladin.h"
#include "PaladinSeal.h"
#include "RaidControl.h"
#include "SanctityAura.h"
#include "SanctityAuraBuff.h"
#include "SealOfCommand.h"
#include "SealOfTheCrusader.h"
#include "Utils/Check.h"

PaladinSpells::PaladinSpells(Paladin* paladin) : CharacterSpells(paladin), paladin(paladin) {
    this->mh_attack = new MainhandAttackPaladin(paladin, this);

    add_spell_group({
        new Consecration(paladin, new_cooldown_control("Consecration", 8.0), 1),
        new Consecration(paladin, new_cooldown_control("Consecration", 8.0), 2),
        new Consecration(paladin, new_cooldown_control("Consecration", 8.0), 3),
        new Consecration(paladin, new_cooldown_control("Consecration", 8.0), 4),
        new Consecration(paladin, new_cooldown_control("Consecration", 8.0), 5),
    });
    add_spell_group({new Judgement(paladin, this, new_cooldown_control("Judgement", 10.0))});
    add_spell_group({mh_attack});
    add_spell_group({new SealOfCommand(paladin, this)});

    auto sanc_buff = static_cast<SanctityAuraBuff*>(pchar->get_raid_control()->get_shared_party_buff("Sanctity Aura", pchar->get_party()));
    if (sanc_buff == nullptr) {
        sanc_buff = new SanctityAuraBuff(paladin);
        sanc_buff->enable_buff();
    }
    add_spell_group({new SanctityAura(paladin, sanc_buff)});

    auto jotc_buff = static_cast<JudgementOfTheCrusader*>(pchar->get_raid_control()->get_shared_raid_buff("Judgement of the Crusader"));
    if (jotc_buff == nullptr) {
        jotc_buff = new JudgementOfTheCrusader(paladin);
        jotc_buff->enable_buff();
    }
    add_spell_group({new SealOfTheCrusader(paladin, jotc_buff)});
}

void PaladinSpells::apply_seal(PaladinSeal* new_seal) {
    if (active_seal != nullptr && active_seal->get_buff()->get_instance_id() != new_seal->get_buff()->get_instance_id())
        active_seal->get_buff()->cancel_buff();

    active_seal = new_seal;
    active_seal->get_buff()->apply_buff();
}

void PaladinSpells::judge_seal(PaladinSeal* seal) {
    check((seal && active_seal && seal->get_instance_id() == active_seal->get_instance_id()), "Tried to judge a seal that was not active");

    if (seal->has_judge_debuff()) {
        judged_seal = active_seal;
    }
    active_seal = nullptr;
}

PaladinSeal* PaladinSpells::get_active_seal() const {
    return this->active_seal;
}

PaladinSeal* PaladinSpells::get_judged_seal() const {
    return this->judged_seal;
}

#pragma once

#include "MagicSchools.h"
#include "Spell.h"

class Random;

enum class ConsumeCharge : bool;

class InstantSpellAttack : public Spell {
public:
    InstantSpellAttack(Character* pchar,
                       const QString& name,
                       const QString& icon,
                       const MagicSchool school,
                       const unsigned min,
                       const unsigned max,
                       const double coefficient,
                       const ConsumeCharge consume_charge,
                       const unsigned innate_threat = 0);
    ~InstantSpellAttack() override;

private:
    const MagicSchool school;
    const unsigned min;
    const unsigned max;
    const double coefficient;
    const ConsumeCharge consume_charge;
    const unsigned innate_threat;
    Random* random;

    void spell_effect() override;
};

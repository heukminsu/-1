#pragma once

#include "CharacterSpells.h"

class BearForm;
class Buff;
class CasterForm;
class CatForm;
class ClearcastingDruid;
class Druid;
class MoonkinForm;
class Proc;
class TigersFuryBuff;
class MainhandAttackDruid;
class Maul;

class DruidSpells : public CharacterSpells {
public:
    DruidSpells(Druid* druid);
    ~DruidSpells() override;

    void mh_auto_attack(const int iteration) override;

    CasterForm* get_caster_form() const;
    BearForm* get_bear_form() const;
    CatForm* get_cat_form() const;
    MoonkinForm* get_moonkin_form() const;
    Buff* get_natures_grace() const;
    Buff* get_cat_form_buff() const;
    Buff* get_bear_form_buff() const;
    Proc* get_omen_of_clarity() const;
    Proc* get_blood_frenzy() const;
    Proc* get_primal_fury() const;
    Proc* get_furor() const;

    bool omen_of_clarity_active() const;

private:
    Druid* druid;

    CasterForm* caster_form;
    BearForm* bear_form;
    CatForm* cat_form;
    MoonkinForm* moonkin_form;
    Buff* natures_grace;
    Buff* cat_form_buff;
    Buff* bear_form_buff;
    Buff* maul_buff;
    TigersFuryBuff* tigers_fury_buff;
    ClearcastingDruid* omen_of_clarity;
    Proc* blood_frenzy;
    Proc* primal_fury;
    Proc* furor;

    Maul* maul;

    void prepare_set_of_combat_iterations_class_specific() override;
};

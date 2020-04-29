#pragma once

#include <QVector>

#include "RegeneratingResource.h"

class Character;

class Mana : public RegeneratingResource {
public:
    Mana(Character*);

    void set_base_mana(const unsigned base_mana);
    unsigned get_max_resource() const override;
    unsigned get_resource_per_tick() override;
    ResourceType get_resource_type() const override;
    double get_tick_rate() const override;
    void increase_max_mana_mod(const unsigned change);
    void decrease_max_mana_mod(const unsigned change);

    friend class Druid;
    friend class Hunter;
    friend class Mage;
    friend class Paladin;
    friend class Priest;
    friend class Shaman;
    friend class Warlock;
    friend class EvocationBuff;
    friend class MageArmorBuff;

private:
    unsigned base_mana {0};
    unsigned mana_per_tick {0};
    double last_use_of_mana {0.0};
    double remainder {0.0};
    double mp5_from_spirit_within_5sr_modifier {0.0};
    bool ignore_5sr {false};
    double bonus_regen_modifier {1.0};

    double max_mana_mod {1.0};
    QVector<int> max_mana_mod_changes;

    void add_next_tick();

    void reset_effect() override;
    void lose_resource_effect() override;
};

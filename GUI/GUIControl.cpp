#include "GUIControl.h"

#include <utility>

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "ActiveItemStatFilterModel.h"
#include "AvailableItemStatFilterModel.h"
#include "BuffBreakdownModel.h"
#include "BuffModel.h"
#include "Character.h"
#include "CharacterDecoder.h"
#include "CharacterEncoder.h"
#include "CharacterLoader.h"
#include "CharacterSpells.h"
#include "CharacterStats.h"
#include "CharacterTalents.h"
#include "ClassStatistics.h"
#include "CombatRoll.h"
#include "ContentPhase.h"
#include "DamageMetersModel.h"
#include "DebuffBreakdownModel.h"
#include "DebuffModel.h"
#include "Druid.h"
#include "Dwarf.h"
#include "EnabledBuffs.h"
#include "EnchantModel.h"
#include "EncounterEnd.h"
#include "EncounterStart.h"
#include "Engine.h"
#include "EngineBreakdownModel.h"
#include "Equipment.h"
#include "EquipmentDb.h"
#include "Faction.h"
#include "GeneralBuffs.h"
#include "Gnome.h"
#include "Human.h"
#include "Hunter.h"
#include "ItemModel.h"
#include "ItemTypeFilterModel.h"
#include "Mage.h"
#include "Mechanics.h"
#include "MeleeDamageAvoidanceBreakdownModel.h"
#include "MeleeDamageBreakdownModel.h"
#include "NightElf.h"
#include "NumberCruncher.h"
#include "Orc.h"
#include "Paladin.h"
#include "Priest.h"
#include "ProcBreakdownModel.h"
#include "Projectile.h"
#include "Quiver.h"
#include "Race.h"
#include "RaidControl.h"
#include "RandomAffixModel.h"
#include "RandomAffixes.h"
#include "ResourceBreakdownModel.h"
#include "Rogue.h"
#include "Rotation.h"
#include "RotationExecutorBreakdownModel.h"
#include "RotationExecutorListModel.h"
#include "RotationFileReader.h"
#include "RotationModel.h"
#include "Rulesets.h"
#include "ScaleResultModel.h"
#include "SetBonusControl.h"
#include "Shaman.h"
#include "SimControl.h"
#include "SimOption.h"
#include "SimScaleModel.h"
#include "SimSettings.h"
#include "SimulationThreadPool.h"
#include "Target.h"
#include "Tauren.h"
#include "TemplateCharacters.h"
#include "ThreatBreakdownModel.h"
#include "Troll.h"
#include "Undead.h"
#include "Warlock.h"
#include "Warrior.h"
#include "Weapon.h"
#include "WeaponModel.h"

GUIControl::GUIControl(QObject* parent) :
    QObject(parent),
    equipment_db(new EquipmentDb()),
    random_affixes_db(new RandomAffixes()),
    character_encoder(new CharacterEncoder()),
    character_decoder(new CharacterDecoder()),
    sim_settings(new SimSettings()),
    target(new Target(63)),
    number_cruncher(new NumberCruncher()),
    supported_classes({"Warrior", "Rogue", "Hunter", "Paladin", "Shaman", "Mage", "Druid", "Warlock"}),
    current_char(nullptr),
    active_stat_filter_model(new ActiveItemStatFilterModel()),
    item_type_filter_model(new ItemTypeFilterModel()),
    rotation_model(new RotationModel()),
    dps_distribution(nullptr),
    mh_enchants(new EnchantModel(EquipmentSlot::MAINHAND, EnchantModel::Permanent)),
    mh_temporary_enchants(new EnchantModel(EquipmentSlot::MAINHAND, EnchantModel::Temporary)),
    oh_enchants(new EnchantModel(EquipmentSlot::OFFHAND, EnchantModel::Permanent)),
    oh_temporary_enchants(new EnchantModel(EquipmentSlot::OFFHAND, EnchantModel::Temporary)),
    ranged_enchants(new EnchantModel(EquipmentSlot::RANGED, EnchantModel::Permanent)),
    head_legs_enchants(new EnchantModel(EquipmentSlot::HEAD, EnchantModel::Permanent)),
    shoulder_enchants(new EnchantModel(EquipmentSlot::SHOULDERS, EnchantModel::Permanent)),
    back_enchants(new EnchantModel(EquipmentSlot::BACK, EnchantModel::Permanent)),
    wrist_enchants(new EnchantModel(EquipmentSlot::WRIST, EnchantModel::Permanent)),
    gloves_enchants(new EnchantModel(EquipmentSlot::GLOVES, EnchantModel::Permanent)),
    chest_enchants(new EnchantModel(EquipmentSlot::CHEST, EnchantModel::Permanent)),
    boots_enchants(new EnchantModel(EquipmentSlot::BOOTS, EnchantModel::Permanent)),
    random_affixes(new RandomAffixModel(-1)),
    sim_in_progress(false),
    active_window("TALENTS") {
    thread_pool = new SimulationThreadPool(equipment_db, random_affixes_db, sim_settings, number_cruncher);
    QObject::connect(thread_pool, &SimulationThreadPool::threads_finished, this, &GUIControl::compile_thread_results);
    QObject::connect(thread_pool, &SimulationThreadPool::update_progress, this, &GUIControl::update_progress);

    this->sim_control = new SimControl(sim_settings, number_cruncher);
    this->sim_scale_model = new SimScaleModel(sim_settings);
    item_model = new ItemModel(equipment_db, item_type_filter_model, active_stat_filter_model);
    weapon_model = new WeaponModel(equipment_db, item_type_filter_model, active_stat_filter_model);
    active_stat_filter_model->set_item_model(item_model);
    active_stat_filter_model->set_weapon_model(weapon_model);
    available_stat_filter_model = new AvailableItemStatFilterModel(active_stat_filter_model);
    buff_model = new BuffModel(sim_settings->get_phase());
    debuff_model = new DebuffModel(sim_settings->get_phase());
    buff_breakdown_model = new BuffBreakdownModel(number_cruncher);
    debuff_breakdown_model = new DebuffBreakdownModel(number_cruncher);
    damage_breakdown_model = new MeleeDamageBreakdownModel(number_cruncher);
    damage_avoidance_breakdown_model = new MeleeDamageAvoidanceBreakdownModel(number_cruncher);
    threat_breakdown_model = new ThreatBreakdownModel(number_cruncher);
    damage_meters_model = new DamageMetersModel(number_cruncher);
    engine_breakdown_model = new EngineBreakdownModel(number_cruncher);
    proc_breakdown_model = new ProcBreakdownModel(number_cruncher);
    resource_breakdown_model = new ResourceBreakdownModel(number_cruncher);
    rotation_executor_list_model = new RotationExecutorListModel(number_cruncher);
    scale_result_model = new ScaleResultModel(number_cruncher);
    random_affixes->set_equipment_db(equipment_db);
    random_affixes->set_random_affixes_db(random_affixes_db);

    for (int i = 0; i < 8; ++i) {
        raid_setup.append(QVector<QVariantMap> {});
        for (int j = 0; j < 5; ++j)
            raid_setup[i].append(QVariantMap());
    }

    races.insert("Dwarf", new Dwarf());
    races.insert("Gnome", new Gnome());
    races.insert("Human", new Human());
    races.insert("Night Elf", new NightElf());
    races.insert("Orc", new Orc());
    races.insert("Tauren", new Tauren());
    races.insert("Troll", new Troll());
    races.insert("Undead", new Undead());

    chars.insert("Druid", load_character("Druid"));
    chars.insert("Hunter", load_character("Hunter"));
    chars.insert("Mage", load_character("Mage"));
    chars.insert("Paladin", load_character("Paladin"));
    chars.insert("Priest", load_character("Priest"));
    chars.insert("Rogue", load_character("Rogue"));
    chars.insert("Shaman", load_character("Shaman"));
    chars.insert("Warlock", load_character("Warlock"));
    chars.insert("Warrior", load_character("Warrior"));

    load_gui_settings();
}

GUIControl::~GUIControl() {
    for (const auto& pchar : chars)
        delete pchar;

    for (const auto& race : races)
        delete race;

    for (const auto& rc : raid_controls)
        delete rc;

    for (const auto& race : raid_member_races)
        delete race;

    delete equipment_db;
    delete random_affixes_db;
    delete item_model;
    delete item_type_filter_model;
    delete active_stat_filter_model;
    delete available_stat_filter_model;
    delete weapon_model;
    delete buff_model;
    delete debuff_model;
    delete rotation_model;
    delete character_encoder;
    delete character_decoder;
    delete thread_pool;
    delete sim_control;
    delete sim_settings;
    delete target;
    delete raid_control;
    delete sim_scale_model;
    delete number_cruncher;
    delete buff_breakdown_model;
    delete debuff_breakdown_model;
    delete damage_breakdown_model;
    delete threat_breakdown_model;
    delete damage_avoidance_breakdown_model;
    delete damage_meters_model;
    delete engine_breakdown_model;
    delete proc_breakdown_model;
    delete resource_breakdown_model;
    delete rotation_executor_list_model;
    delete scale_result_model;
    delete mh_enchants;
    delete mh_temporary_enchants;
    delete oh_enchants;
    delete oh_temporary_enchants;
    delete ranged_enchants;
    delete head_legs_enchants;
    delete shoulder_enchants;
    delete back_enchants;
    delete wrist_enchants;
    delete gloves_enchants;
    delete chest_enchants;
    delete boots_enchants;
}

void GUIControl::set_character(Character* pchar) {
    sim_settings->use_ruleset(Ruleset::Standard, current_char);
    current_char = pchar;
    current_char->set_special_statistics();
    raid_control = raid_controls[current_char->class_name];
    item_type_filter_model->set_character(current_char);
    item_model->set_character(current_char);
    weapon_model->set_character(current_char);
    rotation_model->set_character(current_char);
    selectInformationRotation(0);
    rotation_model->select_rotation();
    character_encoder->set_character(current_char);
    buff_model->set_character(current_char);
    debuff_model->set_character(current_char);

    mh_enchants->set_character(current_char);
    mh_temporary_enchants->set_character(current_char);
    oh_enchants->set_character(current_char);
    oh_temporary_enchants->set_character(current_char);
    ranged_enchants->set_character(current_char);
    head_legs_enchants->set_character(current_char);
    shoulder_enchants->set_character(current_char);
    back_enchants->set_character(current_char);
    wrist_enchants->set_character(current_char);
    gloves_enchants->set_character(current_char);
    chest_enchants->set_character(current_char);
    boots_enchants->set_character(current_char);

    selectDisplayStat(get_attack_mode_as_string());

    raid_setup[0][0] = QVariantMap {{"text", "You"}, {"color", current_char->class_color}, {"selected", true}};

    emit raceChanged();
    emit classChanged();
    emit statsChanged();
    emit rotationChanged();
    emit equipmentChanged();
    emit enchantChanged();
    emit factionChanged();
    emit partyMembersUpdated();
    emit talentsUpdated();
}

void GUIControl::selectClass(const QString& class_name) {
    if (!chars.contains(class_name)) {
        qDebug() << QString("Class %1 not found in char list!").arg(class_name);
        return;
    }

    if (!supported_classes.contains(class_name)) {
        qDebug() << QString("Class %1 not implemented").arg(class_name);
        return;
    }

    set_character(chars[class_name]);
}

void GUIControl::selectRace(const QString& race_name) {
    if (!races.contains(race_name)) {
        qDebug() << QString("Race %1 not found").arg(race_name);
        return;
    }

    if (!current_char->race_available(races[race_name])) {
        qDebug() << QString("Race %1 not available for %2").arg(race_name, current_char->class_name);
        return;
    }

    current_char->set_race(races[race_name]);
    emit raceChanged();
    emit statsChanged();
}

void GUIControl::selectFaction(const int faction) {
    if (this->current_char->get_faction()->get_faction() == faction)
        return;

    if (current_char->class_name == "Shaman" || current_char->class_name == "Paladin") {
        const AvailableFactions::Name target_faction = current_char->get_faction()->get_faction_as_enum();
        set_character(chars["Warrior"]);

        if (current_char->get_faction()->get_faction_as_enum() != target_faction)
            current_char->switch_faction();

        reset_race(current_char);
    } else {
        current_char->switch_faction();
        reset_race(current_char);
    }

    if (current_char->get_faction()->is_alliance()) {
        Weapon* wpn = current_char->get_equipment()->get_mainhand();
        if (wpn != nullptr)
            wpn->clear_windfury();
    }

    current_char->get_equipment()->clear_items_not_available_for_faction();

    buff_model->update_buffs();
    debuff_model->update_debuffs();
    mh_temporary_enchants->set_character(current_char);
    item_model->update_items();
    weapon_model->update_items();

    emit raceChanged();
    emit statsChanged();
    emit factionChanged();
    emit enchantChanged();
    emit equipmentChanged();
    emit talentsUpdated();
}

bool GUIControl::raceAvailable(const QString& race_name) {
    if (!races.contains(race_name)) {
        qDebug() << QString("Race %1 not found").arg(race_name);
        return false;
    }

    return current_char->race_available(races[race_name]);
}

void GUIControl::reset_race(Character* pchar) {
    const QVector<QString>& available_races = this->current_char->get_faction()->get_faction_races();

    for (const auto& available_race : available_races) {
        if (!races.contains(available_race)) {
            qDebug() << QString("Race %1 not valid!").arg(available_race);
            continue;
        }

        if (pchar->race_available(races[available_race])) {
            pchar->set_race(races[available_race]);
            break;
        }
    }

    emit raceChanged();
    emit statsChanged();
}

QString GUIControl::get_creature_type() const {
    return this->current_char->get_target()->get_creature_type_string();
}

void GUIControl::setCreatureType(const QString& creature_type) {
    for (const auto& pchar : chars)
        pchar->change_target_creature_type(creature_type);

    emit creatureTypeChanged();
    emit statsChanged();
}

int GUIControl::get_target_armor() const {
    return current_char->get_target()->get_armor();
}

int GUIControl::get_target_base_armor() const {
    return current_char->get_target()->get_base_armor();
}

void GUIControl::setTargetBaseArmor(const int armor) {
    for (const auto& pchar : chars)
        pchar->get_target()->set_base_armor(armor);

    emit targetUpdated();
}

QString GUIControl::getLeftBackgroundImage() const {
    return current_char->get_talents()->get_background_image("LEFT");
}

QString GUIControl::getMidBackgroundImage() const {
    return current_char->get_talents()->get_background_image("MID");
}

QString GUIControl::getRightBackgroundImage() const {
    return current_char->get_talents()->get_background_image("RIGHT");
}

QString GUIControl::getIcon(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->get_icon(tree_position, talent_position);
}

bool GUIControl::showPosition(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->show_position(tree_position, talent_position);
}

bool GUIControl::showBottomArrow(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->show_bottom_arrow(tree_position, talent_position);
}
bool GUIControl::showRightArrow(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->show_right_arrow(tree_position, talent_position);
}

QString GUIControl::getBottomArrow(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->get_bottom_arrow(tree_position, talent_position);
}

QString GUIControl::getRightArrow(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->get_right_arrow(tree_position, talent_position);
}

bool GUIControl::bottomChildAvailable(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->bottom_child_available(tree_position, talent_position);
}

bool GUIControl::bottomChildActive(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->bottom_child_active(tree_position, talent_position);
}

bool GUIControl::rightChildAvailable(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->right_child_available(tree_position, talent_position);
}

bool GUIControl::rightChildActive(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->right_child_active(tree_position, talent_position);
}

bool GUIControl::isActive(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->is_active(tree_position, talent_position);
}

bool GUIControl::isAvailable(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->is_available(tree_position, talent_position);
}

bool GUIControl::isMaxed(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->is_maxed(tree_position, talent_position);
}

bool GUIControl::hasTalentPointsRemaining() const {
    return current_char->get_talents()->has_talent_points_remaining();
}

QString GUIControl::getRank(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->get_rank(tree_position, talent_position);
}

QString GUIControl::getMaxRank(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->get_max_rank(tree_position, talent_position);
}

void GUIControl::incrementRank(const QString& tree_position, const QString& talent_position) {
    current_char->get_talents()->increment_rank(tree_position, talent_position);
    emit talentsUpdated();
    emit statsChanged();
}

void GUIControl::decrementRank(const QString& tree_position, const QString& talent_position) {
    current_char->get_talents()->decrement_rank(tree_position, talent_position);
    emit talentsUpdated();
    emit statsChanged();
}

QString GUIControl::getRequirements(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->get_requirements(tree_position, talent_position);
}

QString GUIControl::getCurrentRankDescription(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->get_current_rank_description(tree_position, talent_position);
}

QString GUIControl::getNextRankDescription(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->get_next_rank_description(tree_position, talent_position);
}

int GUIControl::get_tree_points(const QString& tree_position) const {
    return current_char->get_talents()->get_tree_points(tree_position);
}

QString GUIControl::get_formatted_talent_allocation() const {
    return QString("%1 / %2 / %3").arg(get_left_talent_tree_points()).arg(get_mid_talent_tree_points()).arg(get_right_talent_tree_points());
}

QString GUIControl::getTreeName(const QString& tree_position) const {
    return current_char->get_talents()->get_tree_name(tree_position);
}

QString GUIControl::getTalentName(const QString& tree_position, const QString& talent_position) const {
    return current_char->get_talents()->get_talent_name(tree_position, talent_position);
}

void GUIControl::maxRank(const QString& tree_position, const QString& talent_position) {
    current_char->get_talents()->increase_to_max_rank(tree_position, talent_position);
    emit talentsUpdated();
    emit statsChanged();
}

void GUIControl::minRank(const QString& tree_position, const QString& talent_position) {
    current_char->get_talents()->decrease_to_min_rank(tree_position, talent_position);
    emit talentsUpdated();
    emit statsChanged();
}

void GUIControl::clearTree(const QString& tree_position) {
    current_char->get_talents()->clear_tree(tree_position);
    emit talentsUpdated();
    emit statsChanged();
}

void GUIControl::setTalentSetup(const int talent_index) {
    current_char->get_talents()->set_current_index(talent_index);
    emit talentsUpdated();
    emit statsChanged();
}

int GUIControl::get_left_talent_tree_points() const {
    return get_tree_points("LEFT");
}

int GUIControl::get_mid_talent_tree_points() const {
    return get_tree_points("MID");
}

int GUIControl::get_right_talent_tree_points() const {
    return get_tree_points("RIGHT");
}

QString GUIControl::get_stats_type_to_display() const {
    return stats_type_to_display;
}

QString GUIControl::get_attack_mode_as_string() const {
    switch (current_char->get_spells()->get_attack_mode()) {
    case MeleeAttack:
        return "MELEE";
    case RangedAttack:
        return "RANGED";
    case MagicAttack:
        return "SPELL";
    }

    return "<unknown";
}

void GUIControl::selectDisplayStat(const QString& attack_mode) {
    stats_type_to_display = attack_mode;

    emit displayStatsTypeChanged();
}

int GUIControl::get_talent_points_remaining() const {
    return current_char->get_talents()->get_talent_points_remaining();
}

QString GUIControl::get_class_color() const {
    return current_char->class_color;
}

QString GUIControl::get_class_name() const {
    return current_char->class_name;
}

QString GUIControl::get_race_name() const {
    return current_char->get_race()->get_name();
}

bool GUIControl::get_is_alliance() const {
    return this->current_char->get_faction()->is_alliance();
}

bool GUIControl::get_is_horde() const {
    return this->current_char->get_faction()->is_horde();
}

unsigned GUIControl::get_strength() const {
    return current_char->get_stats()->get_strength();
}

unsigned GUIControl::get_agility() const {
    return current_char->get_stats()->get_agility();
}

unsigned GUIControl::get_stamina() const {
    return current_char->get_stats()->get_stamina();
}

unsigned GUIControl::get_intellect() const {
    return current_char->get_stats()->get_intellect();
}

unsigned GUIControl::get_spirit() const {
    return current_char->get_stats()->get_spirit();
}

QString GUIControl::get_melee_crit_chance() const {
    auto mh_crit = current_char->get_stats()->get_mh_crit_chance();
    auto oh_crit = current_char->get_stats()->get_oh_crit_chance();

    if (oh_crit > 0 && mh_crit != oh_crit) {
        QString format = "%1% / %2";
        return format.arg(QString::number(static_cast<double>(mh_crit) / 100, 'f', 2), QString::number(static_cast<double>(oh_crit) / 100, 'f', 2));
    }

    return QString::number(static_cast<double>(mh_crit) / 100, 'f', 2);
}

QString GUIControl::get_melee_hit_chance() const {
    return QString::number(static_cast<double>(current_char->get_stats()->get_melee_hit_chance() / 100), 'f', 2);
}

QString GUIControl::get_ranged_crit_chance() const {
    return QString::number(static_cast<double>(current_char->get_stats()->get_ranged_crit_chance()) / 100, 'f', 2);
}

QString GUIControl::get_ranged_hit_chance() const {
    return QString::number(static_cast<double>(current_char->get_stats()->get_ranged_hit_chance() / 100), 'f', 2);
}

unsigned GUIControl::get_melee_attack_power() const {
    return current_char->get_stats()->get_melee_ap();
}

unsigned GUIControl::get_ranged_attack_power() const {
    return current_char->get_stats()->get_ranged_ap();
}

unsigned GUIControl::get_mainhand_wpn_skill() const {
    return current_char->get_mh_wpn_skill();
}

unsigned GUIControl::get_offhand_wpn_skill() const {
    return current_char->get_oh_wpn_skill();
}

unsigned GUIControl::get_ranged_wpn_skill() const {
    return current_char->get_ranged_wpn_skill();
}

unsigned GUIControl::get_spell_power() const {
    // TODO: Must find a way to show school-specific stats in GUI.
    return current_char->get_stats()->get_spell_damage(MagicSchool::Fire);
}

QString GUIControl::get_spell_hit_chance() const {
    // TODO: Must find a way to show school-specific stats in GUI.
    return QString("%1%").arg(QString::number(static_cast<double>(current_char->get_stats()->get_spell_hit_chance(MagicSchool::Fire)) / 100, 'f', 2));
}

QString GUIControl::get_spell_crit_chance() const {
    // TODO: Must find a way to show school-specific stats in GUI.
    return QString("%1%").arg(QString::number(static_cast<double>(current_char->get_stats()->get_spell_crit_chance(MagicSchool::Fire)) / 100, 'f', 2));
}

ItemModel* GUIControl::get_item_model() const {
    return this->item_model;
}

WeaponModel* GUIControl::get_weapon_model() const {
    return this->weapon_model;
}

ItemTypeFilterModel* GUIControl::get_item_type_filter_model() const {
    return this->item_type_filter_model;
}

ActiveItemStatFilterModel* GUIControl::get_active_stat_filter_model() const {
    return this->active_stat_filter_model;
}

AvailableItemStatFilterModel* GUIControl::get_available_stat_filter_model() const {
    return this->available_stat_filter_model;
}

bool GUIControl::getFilterActive(const int filter) const {
    return this->item_type_filter_model->get_filter_active(filter);
}

void GUIControl::toggleSingleFilter(const int filter) {
    this->item_type_filter_model->toggle_single_filter(filter);
    this->item_model->update_items();
    this->weapon_model->update_items();
    emit filtersUpdated();
}

void GUIControl::clearFiltersAndSelectSingle(const int filter) {
    this->item_type_filter_model->clear_filters_and_select_single_filter(filter);
    this->item_model->update_items();
    this->weapon_model->update_items();
    emit filtersUpdated();
}

void GUIControl::selectRangeOfFiltersFromPrevious(const int filter) {
    this->item_type_filter_model->select_range_of_filters(filter);
    this->item_model->update_items();
    this->weapon_model->update_items();
    emit filtersUpdated();
}

RandomAffixModel* GUIControl::get_random_affix_model() const {
    return this->random_affixes;
}

void GUIControl::setRandomAffixesModelId(const int item_id) {
    random_affixes->set_id(item_id);
}

BuffModel* GUIControl::get_buff_model() const {
    return this->buff_model;
}

DebuffModel* GUIControl::get_debuff_model() const {
    return this->debuff_model;
}

void GUIControl::toggleSingleBuff(const QString& buff) {
    buff_model->toggle_single_buff(buff);
    emit statsChanged();
}

void GUIControl::clearBuffsAndSelectSingleBuff(const QString& buff) {
    buff_model->clear_buffs_and_select_single_buff(buff);
    emit statsChanged();
}

void GUIControl::selectRangeOfBuffs(const QString& buff) {
    buff_model->select_range_of_buffs(buff);
    emit statsChanged();
}

bool GUIControl::buffActive(const QString& buff) const {
    return current_char->get_enabled_buffs()->get_general_buffs()->buff_active(buff);
}

void GUIControl::toggleSingleDebuff(const QString& debuff) {
    debuff_model->toggle_single_debuff(debuff);

    emit targetUpdated();
}

void GUIControl::clearDebuffsAndSelectSingleDebuff(const QString& debuff) {
    debuff_model->clear_debuffs_and_select_single_debuff(debuff);

    emit targetUpdated();
}

void GUIControl::selectRangeOfDebuffs(const QString& debuff) {
    debuff_model->select_range_of_debuffs(debuff);

    emit targetUpdated();
}

bool GUIControl::debuffActive(const QString& debuff) const {
    return current_char->get_enabled_buffs()->get_general_buffs()->debuff_active(debuff);
}

void GUIControl::setBuffSetup(const int buff_index) {
    current_char->get_enabled_buffs()->get_general_buffs()->change_setup(buff_index);
    buff_model->update_buffs();
    debuff_model->update_debuffs();
    emit statsChanged();
}

BuffBreakdownModel* GUIControl::get_buff_breakdown_model() const {
    return this->buff_breakdown_model;
}

DebuffBreakdownModel* GUIControl::get_debuff_breakdown_model() const {
    return this->debuff_breakdown_model;
}

EngineBreakdownModel* GUIControl::get_engine_breakdown_model() const {
    return this->engine_breakdown_model;
}

MeleeDamageBreakdownModel* GUIControl::get_dmg_breakdown_model() const {
    return this->damage_breakdown_model;
}

ThreatBreakdownModel* GUIControl::get_thrt_breakdown_model() const {
    return this->threat_breakdown_model;
}

MeleeDamageAvoidanceBreakdownModel* GUIControl::get_dmg_breakdown_avoidance_model() const {
    return this->damage_avoidance_breakdown_model;
}

ProcBreakdownModel* GUIControl::get_proc_breakdown_model() const {
    return this->proc_breakdown_model;
}

ResourceBreakdownModel* GUIControl::get_resource_breakdown_model() const {
    return this->resource_breakdown_model;
}

RotationExecutorBreakdownModel* GUIControl::get_rotation_executor_model() const {
    return this->rotation_executor_list_model->executor_breakdown_model;
}

RotationExecutorListModel* GUIControl::get_rotation_executor_list_model() const {
    return this->rotation_executor_list_model;
}

ScaleResultModel* GUIControl::get_scale_result_model() const {
    return this->scale_result_model;
}

RotationModel* GUIControl::get_rotation_model() const {
    return this->rotation_model;
}

void GUIControl::selectRotation() {
    rotation_model->select_rotation();
    emit rotationChanged();
}

void GUIControl::selectInformationRotation(const int index) {
    if (!rotation_model->set_information_index(index))
        return;

    emit informationRotationChanged();
}

void GUIControl::resetDefaultSettings() {
    setCombatIterationsFullSim(10000);
    setCombatIterationsQuickSim(1000);
    setCombatLength(300);
    setNumThreads(sim_settings->get_num_threads_max());
    setTargetBaseArmor(Mechanics::get_boss_base_armor());
}

QString GUIControl::get_curr_rotation_name() const {
    return current_char->get_spells()->get_rotation()->get_name();
}

QString GUIControl::get_curr_rotation_description() const {
    return current_char->get_spells()->get_rotation()->get_description();
}

QString GUIControl::get_information_rotation_name() const {
    return rotation_model->get_rotation_information_name();
}

QString GUIControl::get_information_rotation_description() const {
    return rotation_model->get_rotation_information_description();
}

void GUIControl::update_displayed_dps_value(const double new_dps_value, const double new_tps_value) {
    double dps_previous = last_personal_sim_result;
    last_personal_sim_result = new_dps_value;
    double delta = ((last_personal_sim_result - dps_previous) / dps_previous);
    QString dps_change = delta > 0 ? "+" : "";
    dps_change += QString::number(((last_personal_sim_result - dps_previous) / dps_previous) * 100, 'f', 1) + "%";

    double tps_previous = last_personal_sim_result_tps;
    last_personal_sim_result_tps = new_tps_value;
    delta = ((last_personal_sim_result_tps - tps_previous) / tps_previous);
    QString tps_change = delta > 0 ? "+" : "";
    tps_change += QString::number(((last_personal_sim_result_tps - tps_previous) / tps_previous) * 100, 'f', 1) + "%";

    QString dps = QString::number(last_personal_sim_result, 'f', 2) + " DPS";
    QString tps = QString::number(new_tps_value, 'f', 2) + " TPS (" + tps_change + ")";
    qDebug() << "Total DPS: " << dps;
    qDebug() << "Total TPS:" << tps;
    simPersonalResultUpdated(dps, dps_change, tps, delta > 0);
}

void GUIControl::update_displayed_raid_dps_value(const double new_dps_value) {
    double previous = last_raid_sim_result;
    last_raid_sim_result = new_dps_value;
    double delta = ((last_raid_sim_result - previous) / previous);
    QString change = delta > 0 ? "+" : "";
    change += QString::number(((last_raid_sim_result - previous) / previous) * 100, 'f', 1) + "%";

    QString dps = QString::number(last_raid_sim_result, 'f', 2);

    qDebug() << "Total Raid DPS:" << dps;

    simRaidResultUpdated(dps, change, delta > 0);
}

void GUIControl::calculate_displayed_dps_value() {
    // Note: intended for local testing to build confidence in thread results
    const int total_duration = sim_settings->get_combat_iterations_full_sim() * sim_settings->get_combat_length();
    update_displayed_dps_value(double(current_char->get_statistics()->get_total_personal_damage_dealt()) / total_duration,
                               double(current_char->get_statistics()->get_total_personal_threat_dealt()) / total_duration);
}

void GUIControl::runQuickSim() {
    if (sim_in_progress)
        return;
    if (current_char->get_spells()->get_attack_mode() == AttackMode::MeleeAttack && current_char->get_equipment()->get_mainhand() == nullptr)
        return;
    if (current_char->get_spells()->get_attack_mode() == AttackMode::RangedAttack && current_char->get_equipment()->get_ranged() == nullptr)
        return;

    sim_in_progress = true;

    QVector<QString> setup_strings;
    raid_setup[0][0]["setup_string"] = CharacterEncoder(current_char).get_current_setup_string();

    for (const auto& party : raid_setup) {
        for (const auto& party_member : party) {
            if (party_member.contains("setup_string"))
                setup_strings.append(party_member["setup_string"].toString());
        }
    }

    thread_pool->run_sim(setup_strings, false, sim_settings->get_combat_iterations_quick_sim(), 1);

    emit simProgressChanged();
}

void GUIControl::runFullSim() {
    if (sim_in_progress)
        return;
    if (current_char->get_spells()->get_attack_mode() == AttackMode::MeleeAttack && current_char->get_equipment()->get_mainhand() == nullptr)
        return;
    if (current_char->get_spells()->get_attack_mode() == AttackMode::RangedAttack && current_char->get_equipment()->get_ranged() == nullptr)
        return;

    sim_in_progress = true;

    QVector<QString> setup_strings;
    raid_setup[0][0]["setup_string"] = CharacterEncoder(current_char).get_current_setup_string();

    for (const auto& party : raid_setup) {
        for (const auto& party_member : party) {
            if (party_member.contains("setup_string"))
                setup_strings.append(party_member["setup_string"].toString());
        }
    }

    const int num_stat_weights = 1 + sim_settings->get_active_options().size();
    thread_pool->run_sim(setup_strings, true, sim_settings->get_combat_iterations_full_sim(), num_stat_weights);

    emit simProgressChanged();
}

void GUIControl::compile_thread_results() {
    scale_result_model->update_statistics();
    buff_breakdown_model->update_statistics();
    debuff_breakdown_model->update_statistics();
    damage_breakdown_model->update_statistics();
    threat_breakdown_model->update_statistics();
    damage_avoidance_breakdown_model->update_statistics();
    engine_breakdown_model->update_statistics();
    proc_breakdown_model->update_statistics();
    resource_breakdown_model->update_statistics();
    rotation_executor_list_model->update_statistics();
    damage_meters_model->update_statistics();
    last_engine_handled_events_per_second = engine_breakdown_model->events_handled_per_second();
    update_displayed_dps_value(number_cruncher->get_personal_dps(SimOption::Name::NoScale), number_cruncher->get_personal_tps(SimOption::Name::NoScale));
    update_displayed_raid_dps_value(number_cruncher->get_raid_dps());
    dps_distribution = number_cruncher->get_dps_distribution();
    number_cruncher->reset();
    sim_in_progress = false;
    sim_percent_completed = 0.0;
    emit simProgressChanged();
    emit combatProgressChanged();
    emit statisticsReady();
}

QString GUIControl::get_handled_events_per_second() const {
    return QString::number(last_engine_handled_events_per_second, 'f', 0);
}

QString GUIControl::get_min_dps() const {
    if (dps_distribution == nullptr)
        return "0.0";

    return QString::number(dps_distribution->min_dps, 'f', 2);
}

QString GUIControl::get_max_dps() const {
    if (dps_distribution == nullptr)
        return "0.0";

    return QString::number(dps_distribution->max_dps, 'f', 2);
}

QString GUIControl::get_standard_deviation() const {
    if (dps_distribution == nullptr)
        return "0.0";

    return QString::number(dps_distribution->standard_deviation, 'f', 2);
}

QString GUIControl::get_confidence_interval() const {
    if (dps_distribution == nullptr)
        return "0.0";

    return QString::number(dps_distribution->confidence_interval, 'f', 2);
}

int GUIControl::get_combat_iterations_full_sim() const {
    return sim_settings->get_combat_iterations_full_sim();
}

int GUIControl::get_combat_iterations_quick_sim() const {
    return sim_settings->get_combat_iterations_quick_sim();
}

int GUIControl::get_combat_length() const {
    return sim_settings->get_combat_length();
}

int GUIControl::get_num_threads() const {
    return sim_settings->get_num_threads_current();
}

int GUIControl::get_max_threads() const {
    return sim_settings->get_num_threads_max();
}

void GUIControl::setCombatIterationsFullSim(const int iterations) {
    sim_settings->set_combat_iterations_full_sim(iterations);
    emit combatIterationsChanged();
}

void GUIControl::setCombatIterationsQuickSim(const int iterations) {
    sim_settings->set_combat_iterations_quick_sim(iterations);
    emit combatIterationsChanged();
}

void GUIControl::setCombatLength(const int length) {
    sim_settings->set_combat_length(length);
    emit combatLengthChanged();
}

void GUIControl::setNumThreads(const int threads) {
    if (thread_pool->sim_running())
        return;

    sim_settings->set_num_threads(threads);
    thread_pool->scale_number_of_threads();

    emit numThreadsChanged();
}

void GUIControl::selectRuleset(const int ruleset) {
    sim_settings->use_ruleset(static_cast<Ruleset>(ruleset), current_char);
    emit statsChanged();
}

SimScaleModel* GUIControl::get_sim_scale_model() const {
    return this->sim_scale_model;
}

int GUIControl::get_combat_progress() const {
    return static_cast<int>(round(sim_percent_completed * 100));
}

bool GUIControl::get_sim_in_progress() const {
    return sim_in_progress;
}

void GUIControl::selectPartyMember(const int party, const int member) {
    current_party = party;
    current_member = member;
    emit selectedPartyMemberChanged();
}

void GUIControl::clearPartyMember(const int party, const int member) {
    if (party == 1 && member == 1)
        return;

    raid_setup[party - 1][member - 1].clear();
    emit partyMembersUpdated();
}

QVariantMap GUIControl::partyMemberInfo(const int party, const int member) {
    raid_setup[party - 1][member - 1]["selected"] = (party == this->current_party && member == this->current_member);
    return raid_setup[party - 1][member - 1];
}

void GUIControl::selectTemplateCharacter(QString template_char) {
    if (current_party == 1 && current_member == 1)
        return;

    TemplateCharacterInfo info = TemplateCharacters::template_character_info(template_char);
    const QString color = chars[info.class_name]->class_color;
    const QString setup_string = info.setup_string.arg(static_cast<int>(sim_settings->get_phase())).arg(current_party - 1).arg(current_member - 1);

    raid_setup[current_party - 1][current_member - 1] = QVariantMap {{"text", template_char}, {"color", color}, {"setup_string", setup_string}};

    emit partyMembersUpdated();
}

DamageMetersModel* GUIControl::get_damage_meters_model() {
    return this->damage_meters_model;
}

QString GUIControl::get_mainhand_icon() const {
    if (current_char->get_equipment()->get_mainhand() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_mainhand()->get_value("icon");
    return "";
}

QString GUIControl::get_offhand_icon() const {
    if (current_char->get_equipment()->get_offhand() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_offhand()->get_value("icon");
    return "";
}

QString GUIControl::get_ranged_icon() const {
    if (current_char->get_equipment()->get_ranged() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_ranged()->get_value("icon");
    return "";
}

QString GUIControl::get_head_icon() const {
    if (current_char->get_equipment()->get_head() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_head()->get_value("icon");
    return "";
}

QString GUIControl::get_neck_icon() const {
    if (current_char->get_equipment()->get_neck() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_neck()->get_value("icon");
    return "";
}

QString GUIControl::get_shoulders_icon() const {
    if (current_char->get_equipment()->get_shoulders() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_shoulders()->get_value("icon");
    return "";
}

QString GUIControl::get_back_icon() const {
    if (current_char->get_equipment()->get_back() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_back()->get_value("icon");
    return "";
}

QString GUIControl::get_chest_icon() const {
    if (current_char->get_equipment()->get_chest() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_chest()->get_value("icon");
    return "";
}

QString GUIControl::get_wrist_icon() const {
    if (current_char->get_equipment()->get_wrist() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_wrist()->get_value("icon");
    return "";
}

QString GUIControl::get_gloves_icon() const {
    if (current_char->get_equipment()->get_gloves() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_gloves()->get_value("icon");
    return "";
}

QString GUIControl::get_belt_icon() const {
    if (current_char->get_equipment()->get_belt() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_belt()->get_value("icon");
    return "";
}

QString GUIControl::get_legs_icon() const {
    if (current_char->get_equipment()->get_legs() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_legs()->get_value("icon");
    return "";
}

QString GUIControl::get_boots_icon() const {
    if (current_char->get_equipment()->get_boots() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_boots()->get_value("icon");
    return "";
}

QString GUIControl::get_ring1_icon() const {
    if (current_char->get_equipment()->get_ring1() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_ring1()->get_value("icon");
    return "";
}

QString GUIControl::get_ring2_icon() const {
    if (current_char->get_equipment()->get_ring2() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_ring2()->get_value("icon");
    return "";
}

QString GUIControl::get_trinket1_icon() const {
    if (current_char->get_equipment()->get_trinket1() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_trinket1()->get_value("icon");
    return "";
}

QString GUIControl::get_trinket2_icon() const {
    if (current_char->get_equipment()->get_trinket2() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_trinket2()->get_value("icon");
    return "";
}

QString GUIControl::get_projectile_icon() const {
    if (current_char->get_equipment()->get_projectile() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_projectile()->get_value("icon");
    return "";
}

QString GUIControl::get_relic_icon() const {
    if (current_char->get_equipment()->get_relic() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_relic()->get_value("icon");
    return "";
}

QString GUIControl::get_quiver_icon() const {
    if (current_char->get_equipment()->get_quiver() != nullptr)
        return "Assets/items/" + current_char->get_equipment()->get_quiver()->get_value("icon");
    return "";
}

void GUIControl::selectSlot(const QString& slot_string) {
    int slot = get_slot_int(slot_string);

    if (slot == -1 || (slot == ItemSlots::PROJECTILE && current_char->class_name != "Hunter"))
        return;

    if (slot == ItemSlots::QUIVER && current_char->class_name != "Hunter")
        return;

    item_type_filter_model->set_item_slot(slot);

    switch (slot) {
    case ItemSlots::MAINHAND:
    case ItemSlots::OFFHAND:
    case ItemSlots::RANGED:
        weapon_model->setSlot(slot);
        break;
    default:
        item_model->setSlot(slot);
        break;
    }

    emit equipmentSlotSelected();
}

bool GUIControl::hasItemEquipped(const QString& slot_string) const {
    if (slot_string == "MAINHAND")
        return current_char->get_equipment()->get_mainhand() != nullptr;
    if (slot_string == "OFFHAND")
        return current_char->get_equipment()->get_offhand() != nullptr;
    if (slot_string == "RANGED")
        return current_char->get_equipment()->get_ranged() != nullptr;
    if (slot_string == "HEAD")
        return current_char->get_equipment()->get_head() != nullptr;
    if (slot_string == "NECK")
        return current_char->get_equipment()->get_neck() != nullptr;
    if (slot_string == "SHOULDERS")
        return current_char->get_equipment()->get_shoulders() != nullptr;
    if (slot_string == "BACK")
        return current_char->get_equipment()->get_back() != nullptr;
    if (slot_string == "CHEST")
        return current_char->get_equipment()->get_chest() != nullptr;
    if (slot_string == "WRIST")
        return current_char->get_equipment()->get_wrist() != nullptr;
    if (slot_string == "GLOVES")
        return current_char->get_equipment()->get_gloves() != nullptr;
    if (slot_string == "BELT")
        return current_char->get_equipment()->get_belt() != nullptr;
    if (slot_string == "LEGS")
        return current_char->get_equipment()->get_legs() != nullptr;
    if (slot_string == "BOOTS")
        return current_char->get_equipment()->get_boots() != nullptr;
    if (slot_string == "RING1")
        return current_char->get_equipment()->get_ring1() != nullptr;
    if (slot_string == "RING2")
        return current_char->get_equipment()->get_ring2() != nullptr;
    if (slot_string == "TRINKET1")
        return current_char->get_equipment()->get_trinket1() != nullptr;
    if (slot_string == "TRINKET2")
        return current_char->get_equipment()->get_trinket2() != nullptr;
    if (slot_string == "RELIC")
        return current_char->get_equipment()->get_relic() != nullptr;
    if (slot_string == "QUIVER")
        return current_char->get_equipment()->get_quiver() != nullptr;

    return false;
}

bool GUIControl::hasEnchant(const QString& slot_string) const {
    if (!hasItemEquipped(slot_string))
        return false;

    if (slot_string == "MAINHAND")
        return current_char->get_equipment()->get_mainhand()->has_enchant();
    if (slot_string == "OFFHAND")
        return current_char->get_equipment()->get_offhand()->has_enchant();
    if (slot_string == "RANGED")
        return current_char->get_equipment()->get_ranged()->has_enchant();
    if (slot_string == "HEAD")
        return current_char->get_equipment()->get_head()->has_enchant();
    if (slot_string == "SHOULDERS")
        return current_char->get_equipment()->get_shoulders()->has_enchant();
    if (slot_string == "BACK")
        return current_char->get_equipment()->get_back()->has_enchant();
    if (slot_string == "CHEST")
        return current_char->get_equipment()->get_chest()->has_enchant();
    if (slot_string == "WRIST")
        return current_char->get_equipment()->get_wrist()->has_enchant();
    if (slot_string == "GLOVES")
        return current_char->get_equipment()->get_gloves()->has_enchant();
    if (slot_string == "LEGS")
        return current_char->get_equipment()->get_legs()->has_enchant();
    if (slot_string == "BOOTS")
        return current_char->get_equipment()->get_boots()->has_enchant();

    return false;
}

bool GUIControl::hasTemporaryEnchant(const QString& slot_string) const {
    if (!hasItemEquipped(slot_string))
        return false;

    if (slot_string == "MAINHAND")
        return current_char->get_equipment()->get_mainhand()->has_temporary_enchant();
    if (slot_string == "OFFHAND")
        return current_char->get_equipment()->get_offhand()->has_temporary_enchant();

    return false;
}

QString GUIControl::getEnchantEffect(const QString& slot_string) const {
    if (slot_string == "MAINHAND")
        return current_char->get_equipment()->get_mainhand()->get_enchant_effect();
    if (slot_string == "OFFHAND")
        return current_char->get_equipment()->get_offhand()->get_enchant_effect();
    if (slot_string == "RANGED")
        return current_char->get_equipment()->get_ranged()->get_enchant_effect();
    if (slot_string == "HEAD")
        return current_char->get_equipment()->get_head()->get_enchant_effect();
    if (slot_string == "SHOULDERS")
        return current_char->get_equipment()->get_shoulders()->get_enchant_effect();
    if (slot_string == "BACK")
        return current_char->get_equipment()->get_back()->get_enchant_effect();
    if (slot_string == "CHEST")
        return current_char->get_equipment()->get_chest()->get_enchant_effect();
    if (slot_string == "WRIST")
        return current_char->get_equipment()->get_wrist()->get_enchant_effect();
    if (slot_string == "GLOVES")
        return current_char->get_equipment()->get_gloves()->get_enchant_effect();
    if (slot_string == "LEGS")
        return current_char->get_equipment()->get_legs()->get_enchant_effect();
    if (slot_string == "BOOTS")
        return current_char->get_equipment()->get_boots()->get_enchant_effect();

    return "";
}

QString GUIControl::getTemporaryEnchantEffect(const QString& slot_string) const {
    if (slot_string == "MAINHAND")
        return current_char->get_equipment()->get_mainhand()->get_temporary_enchant_effect();
    if (slot_string == "OFFHAND")
        return current_char->get_equipment()->get_offhand()->get_temporary_enchant_effect();

    return "";
}

void GUIControl::applyEnchant(const QString& slot_string, const int enchant_name) {
    if (slot_string == "MAINHAND")
        current_char->get_equipment()->get_mainhand()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char,
                                                                     WeaponSlots::MAINHAND);
    if (slot_string == "OFFHAND")
        current_char->get_equipment()->get_offhand()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char, WeaponSlots::OFFHAND);
    if (slot_string == "RANGED")
        current_char->get_equipment()->get_ranged()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char, WeaponSlots::RANGED);
    if (slot_string == "HEAD")
        current_char->get_equipment()->get_head()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char);
    if (slot_string == "SHOULDERS")
        current_char->get_equipment()->get_shoulders()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char);
    if (slot_string == "BACK")
        current_char->get_equipment()->get_back()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char);
    if (slot_string == "CHEST")
        current_char->get_equipment()->get_chest()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char);
    if (slot_string == "WRIST")
        current_char->get_equipment()->get_wrist()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char);
    if (slot_string == "GLOVES")
        current_char->get_equipment()->get_gloves()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char);
    if (slot_string == "LEGS")
        current_char->get_equipment()->get_legs()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char);
    if (slot_string == "BOOTS")
        current_char->get_equipment()->get_boots()->apply_enchant(static_cast<EnchantName::Name>(enchant_name), current_char);

    emit enchantChanged();
    emit statsChanged();
}

void GUIControl::applyTemporaryEnchant(const QString& slot_string, const int enchant_name) {
    if (slot_string == "MAINHAND")
        current_char->get_equipment()->get_mainhand()->apply_temporary_enchant(static_cast<EnchantName::Name>(enchant_name), current_char,
                                                                               EnchantSlot::MAINHAND);
    if (slot_string == "OFFHAND")
        current_char->get_equipment()->get_offhand()->apply_temporary_enchant(static_cast<EnchantName::Name>(enchant_name), current_char,
                                                                              EnchantSlot::OFFHAND);

    emit enchantChanged();
    emit statsChanged();
}

void GUIControl::clearEnchant(const QString& slot_string) {
    if (slot_string == "MAINHAND")
        current_char->get_equipment()->get_mainhand()->clear_enchant();
    if (slot_string == "OFFHAND")
        current_char->get_equipment()->get_offhand()->clear_enchant();
    if (slot_string == "RANGED")
        current_char->get_equipment()->get_ranged()->clear_enchant();
    if (slot_string == "HEAD")
        current_char->get_equipment()->get_head()->clear_enchant();
    if (slot_string == "SHOULDERS")
        current_char->get_equipment()->get_shoulders()->clear_enchant();
    if (slot_string == "BACK")
        current_char->get_equipment()->get_back()->clear_enchant();
    if (slot_string == "CHEST")
        current_char->get_equipment()->get_chest()->clear_enchant();
    if (slot_string == "WRIST")
        current_char->get_equipment()->get_wrist()->clear_enchant();
    if (slot_string == "GLOVES")
        current_char->get_equipment()->get_gloves()->clear_enchant();
    if (slot_string == "LEGS")
        current_char->get_equipment()->get_legs()->clear_enchant();
    if (slot_string == "BOOTS")
        current_char->get_equipment()->get_boots()->clear_enchant();

    emit enchantChanged();
    emit statsChanged();
}

void GUIControl::clearTemporaryEnchant(const QString& slot_string) {
    if (slot_string == "MAINHAND")
        current_char->get_equipment()->get_mainhand()->clear_temporary_enchant();
    if (slot_string == "OFFHAND")
        current_char->get_equipment()->get_offhand()->clear_temporary_enchant();

    emit enchantChanged();
    emit statsChanged();
}

EnchantModel* GUIControl::get_mh_enchant_model() const {
    return this->mh_enchants;
}

EnchantModel* GUIControl::get_mh_temporary_enchant_model() const {
    return this->mh_temporary_enchants;
}

EnchantModel* GUIControl::get_oh_enchant_model() const {
    return this->oh_enchants;
}

EnchantModel* GUIControl::get_oh_temporary_enchant_model() const {
    return this->oh_temporary_enchants;
}

EnchantModel* GUIControl::get_ranged_enchant_model() const {
    return this->ranged_enchants;
}

EnchantModel* GUIControl::get_head_legs_enchant_model() const {
    return this->head_legs_enchants;
}

EnchantModel* GUIControl::get_shoulder_enchant_model() const {
    return this->shoulder_enchants;
}

EnchantModel* GUIControl::get_back_enchant_model() const {
    return this->back_enchants;
}

EnchantModel* GUIControl::get_wrist_enchant_model() const {
    return this->wrist_enchants;
}

EnchantModel* GUIControl::get_gloves_enchant_model() const {
    return this->gloves_enchants;
}

EnchantModel* GUIControl::get_chest_enchant_model() const {
    return this->chest_enchants;
}

EnchantModel* GUIControl::get_boots_enchant_model() const {
    return this->boots_enchants;
}

void GUIControl::setSlot(const QString& slot_string, const int item_id, const uint affix_id) {
    RandomAffix* affix = random_affixes_db->get_affix(affix_id);

    if (slot_string == "MAINHAND") {
        current_char->get_equipment()->set_mainhand(item_id, affix);
        mh_enchants->update_enchants();
        mh_temporary_enchants->update_enchants();
    }
    if (slot_string == "OFFHAND") {
        current_char->get_equipment()->set_offhand(item_id, affix);
        oh_temporary_enchants->update_enchants();
    }
    if (slot_string == "RANGED")
        current_char->get_equipment()->set_ranged(item_id, affix);
    if (slot_string == "HEAD")
        current_char->get_equipment()->set_head(item_id, affix);
    if (slot_string == "NECK")
        current_char->get_equipment()->set_neck(item_id, affix);
    if (slot_string == "SHOULDERS")
        current_char->get_equipment()->set_shoulders(item_id, affix);
    if (slot_string == "BACK")
        current_char->get_equipment()->set_back(item_id, affix);
    if (slot_string == "CHEST")
        current_char->get_equipment()->set_chest(item_id, affix);
    if (slot_string == "WRIST")
        current_char->get_equipment()->set_wrist(item_id, affix);
    if (slot_string == "GLOVES")
        current_char->get_equipment()->set_gloves(item_id, affix);
    if (slot_string == "BELT")
        current_char->get_equipment()->set_belt(item_id, affix);
    if (slot_string == "LEGS")
        current_char->get_equipment()->set_legs(item_id, affix);
    if (slot_string == "BOOTS")
        current_char->get_equipment()->set_boots(item_id, affix);
    if (slot_string == "RING1")
        current_char->get_equipment()->set_ring1(item_id, affix);
    if (slot_string == "RING2")
        current_char->get_equipment()->set_ring2(item_id, affix);
    if (slot_string == "TRINKET1")
        current_char->get_equipment()->set_trinket1(item_id);
    if (slot_string == "TRINKET2")
        current_char->get_equipment()->set_trinket2(item_id);
    if (slot_string == "PROJECTILE")
        current_char->get_equipment()->set_projectile(item_id);
    if (slot_string == "RELIC")
        current_char->get_equipment()->set_relic(item_id);
    if (slot_string == "QUIVER")
        current_char->get_equipment()->set_quiver(item_id);

    emit equipmentChanged();
    emit statsChanged();
    emit enchantChanged();
}

void GUIControl::clearSlot(const QString& slot_string) {
    if (slot_string == "MAINHAND")
        current_char->get_equipment()->clear_mainhand();
    if (slot_string == "OFFHAND")
        current_char->get_equipment()->clear_offhand();
    if (slot_string == "RANGED")
        current_char->get_equipment()->clear_ranged();
    if (slot_string == "HEAD")
        current_char->get_equipment()->clear_head();
    if (slot_string == "NECK")
        current_char->get_equipment()->clear_neck();
    if (slot_string == "SHOULDERS")
        current_char->get_equipment()->clear_shoulders();
    if (slot_string == "BACK")
        current_char->get_equipment()->clear_back();
    if (slot_string == "CHEST")
        current_char->get_equipment()->clear_chest();
    if (slot_string == "WRIST")
        current_char->get_equipment()->clear_wrist();
    if (slot_string == "GLOVES")
        current_char->get_equipment()->clear_gloves();
    if (slot_string == "BELT")
        current_char->get_equipment()->clear_belt();
    if (slot_string == "LEGS")
        current_char->get_equipment()->clear_legs();
    if (slot_string == "BOOTS")
        current_char->get_equipment()->clear_boots();
    if (slot_string == "RING1")
        current_char->get_equipment()->clear_ring1();
    if (slot_string == "RING2")
        current_char->get_equipment()->clear_ring2();
    if (slot_string == "TRINKET1")
        current_char->get_equipment()->clear_trinket1();
    if (slot_string == "TRINKET2")
        current_char->get_equipment()->clear_trinket2();
    if (slot_string == "PROJECTILE")
        current_char->get_equipment()->clear_projectile();
    if (slot_string == "RELIC")
        current_char->get_equipment()->clear_relic();
    if (slot_string == "QUIVER")
        current_char->get_equipment()->clear_quiver();

    emit equipmentChanged();
    emit statsChanged();
    emit enchantChanged();
}

void GUIControl::setEquipmentSetup(const int equipment_index) {
    current_char->get_stats()->get_equipment()->change_setup(equipment_index);
    emit equipmentChanged();
    emit statsChanged();
    emit enchantChanged();
}

void GUIControl::setPhase(const int phase_int) {
    Content::Phase phase = Content::get_phase(phase_int);
    sim_settings->set_phase(phase);
    weapon_model->set_phase(phase);
    item_model->set_phase(phase);
    buff_model->set_phase(phase);
    debuff_model->set_phase(phase);

    current_char->get_stats()->get_equipment()->reequip_items();
    emit equipmentChanged();
    emit statsChanged();
    emit enchantChanged();
}

QString GUIControl::getDescriptionForPhase(const int phase) {
    return Content::get_description_for_phase(static_cast<Content::Phase>(phase));
}

QVariantList GUIControl::getTooltip(const QString& slot_string) {
    Item* item = nullptr;

    if (slot_string == "MAINHAND")
        item = current_char->get_equipment()->get_mainhand();
    if (slot_string == "OFFHAND")
        item = current_char->get_equipment()->get_offhand();
    if (slot_string == "RANGED")
        item = current_char->get_equipment()->get_ranged();
    if (slot_string == "HEAD")
        item = current_char->get_equipment()->get_head();
    if (slot_string == "NECK")
        item = current_char->get_equipment()->get_neck();
    if (slot_string == "SHOULDERS")
        item = current_char->get_equipment()->get_shoulders();
    if (slot_string == "BACK")
        item = current_char->get_equipment()->get_back();
    if (slot_string == "CHEST")
        item = current_char->get_equipment()->get_chest();
    if (slot_string == "WRIST")
        item = current_char->get_equipment()->get_wrist();
    if (slot_string == "GLOVES")
        item = current_char->get_equipment()->get_gloves();
    if (slot_string == "BELT")
        item = current_char->get_equipment()->get_belt();
    if (slot_string == "LEGS")
        item = current_char->get_equipment()->get_legs();
    if (slot_string == "BOOTS")
        item = current_char->get_equipment()->get_boots();
    if (slot_string == "RING1")
        item = current_char->get_equipment()->get_ring1();
    if (slot_string == "RING2")
        item = current_char->get_equipment()->get_ring2();
    if (slot_string == "TRINKET1")
        item = current_char->get_equipment()->get_trinket1();
    if (slot_string == "TRINKET2")
        item = current_char->get_equipment()->get_trinket2();
    if (slot_string == "PROJECTILE")
        item = current_char->get_equipment()->get_projectile();
    if (slot_string == "RELIC")
        item = current_char->get_equipment()->get_relic();
    if (slot_string == "QUIVER")
        item = current_char->get_equipment()->get_quiver();

    return get_tooltip_from_item(item);
}

QVariantList GUIControl::getTooltip(const int item_id) {
    Item* item = this->equipment_db->get_item(item_id);
    return get_tooltip_from_item(item);
}

void GUIControl::set_weapon_tooltip(Item*& item, QString& slot, QString type, QString& dmg_range, QString& wpn_speed, QString& dps) {
    slot = std::move(type);
    auto wpn = static_cast<Weapon*>(item);
    dmg_range = QString("%1 - %2 Damage").arg(QString::number(wpn->get_min_dmg()), QString::number(wpn->get_max_dmg()));
    dps = QString("(%1 damage per second)").arg(QString::number(wpn->get_wpn_dps(), 'f', 1));
    wpn_speed = "Speed " + QString::number(wpn->get_base_weapon_speed(), 'f', 2);
}

void GUIControl::set_projectile_tooltip(Item* item, QString& slot, QString& dps) {
    slot = get_initial_upper_case_rest_lower_case(slot);

    auto projectile = static_cast<Projectile*>(item);
    dps = QString("Adds %1 damage per second").arg(QString::number(projectile->get_projectile_dps(), 'f', 1));
}

void GUIControl::set_class_restriction_tooltip(Item*& item, QString& restriction) {
    QVector<QString> restrictions;

    if (item->get_value("RESTRICTED_TO_WARRIOR") != "")
        restrictions.append(QString("<font color=\"%1\">%2</font>").arg(chars["Warrior"]->class_color, chars["Warrior"]->class_name));
    if (item->get_value("RESTRICTED_TO_PALADIN") != "")
        restrictions.append(QString("<font color=\"%1\">%2</font>").arg(chars["Paladin"]->class_color, chars["Paladin"]->class_name));
    if (item->get_value("RESTRICTED_TO_ROGUE") != "")
        restrictions.append(QString("<font color=\"%1\">%2</font>").arg(chars["Rogue"]->class_color, chars["Rogue"]->class_name));
    if (item->get_value("RESTRICTED_TO_HUNTER") != "")
        restrictions.append(QString("<font color=\"%1\">%2</font>").arg(chars["Hunter"]->class_color, chars["Hunter"]->class_name));
    if (item->get_value("RESTRICTED_TO_SHAMAN") != "")
        restrictions.append(QString("<font color=\"%1\">%2</font>").arg(chars["Shaman"]->class_color, chars["Shaman"]->class_name));
    if (item->get_value("RESTRICTED_TO_DRUID") != "")
        restrictions.append(QString("<font color=\"%1\">%2</font>").arg(chars["Druid"]->class_color, chars["Druid"]->class_name));
    if (item->get_value("RESTRICTED_TO_MAGE") != "")
        restrictions.append(QString("<font color=\"%1\">%2</font>").arg(chars["Mage"]->class_color, chars["Mage"]->class_name));
    if (item->get_value("RESTRICTED_TO_PRIEST") != "")
        restrictions.append(QString("<font color=\"%1\">%2</font>").arg(chars["Priest"]->class_color, chars["Priest"]->class_name));
    if (item->get_value("RESTRICTED_TO_WARLOCK") != "")
        restrictions.append(QString("<font color=\"%1\">%2</font>").arg(chars["Warlock"]->class_color, chars["Warlock"]->class_name));

    if (restrictions.empty())
        return;

    for (const auto& i : restrictions) {
        if (restriction == "")
            restriction = "Classes: ";
        else
            restriction += ", ";
        restriction += i;
    }
}

void GUIControl::set_set_bonus_tooltip(Item* item, QVariantList& tooltip) const {
    SetBonusControl* set_bonuses = current_char->get_equipment()->get_set_bonus_control();
    const int item_id = item->item_id;

    if (!set_bonuses->is_set_item(item_id)) {
        tooltip.append(false);
        return;
    }
    tooltip.append(true);

    QString set_name = set_bonuses->get_set_name(item_id);
    QVector<QPair<QString, bool>> item_names_in_set = set_bonuses->get_item_names_in_set(item_id);

    int num_equipped_set_items = set_bonuses->get_num_equipped_pieces_for_set(set_name);
    tooltip.append(QString("%1 (%2/%3)").arg(set_name).arg(num_equipped_set_items).arg(item_names_in_set.size()));

    tooltip.append(item_names_in_set.size());
    for (const auto& item_name : item_names_in_set) {
        QString font_color = item_name.second ? "#ffd100" : "#727171";
        tooltip.append(QString("<font color=\"%1\">%2</font>").arg(font_color, item_name.first));
    }

    QVector<QPair<QString, bool>> set_bonus_tooltips = set_bonuses->get_set_bonus_tooltips(item_id);
    tooltip.append(set_bonus_tooltips.size());
    for (const auto& bonus_tooltip : set_bonus_tooltips) {
        QString font_color = bonus_tooltip.second ? "#1eff00" : "#727171";
        tooltip.append(QString("<font color=\"%1\">%2</font>").arg(font_color, bonus_tooltip.first));
    }
}

QString GUIControl::get_initial_upper_case_rest_lower_case(const QString& string) const {
    return QString(string[0]) + QString(string.right(string.size() - 1)).toLower();
}

QString GUIControl::get_sim_progress_string() const {
    return sim_in_progress ? "Running..." : "Click me!";
}

QString GUIControl::getStartWindow() const {
    return active_window;
}

void GUIControl::changeActiveWindow(const QString& active_window) {
    this->active_window = active_window;
}

int GUIControl::getCurrentRuleset() const {
    return sim_settings->get_ruleset();
}

int GUIControl::getCurrentCreatureType() const {
    return current_char->get_target()->get_creature_type();
}

int GUIControl::getContentPhase() const {
    return static_cast<int>(sim_settings->get_phase());
}

void GUIControl::update_progress(double percent) {
    this->sim_percent_completed = percent;
    emit combatProgressChanged();
}

Character* GUIControl::load_character(const QString& class_name) {
    QFile file(QString("Saves/%1-setup.xml").arg(class_name));

    Character* pchar = get_new_character(class_name);

    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&file);

        reader.readNextStartElement();
        for (int i = 0; i < 3; ++i) {
            reader.readNextStartElement();

            CharacterDecoder decoder;
            decoder.initialize(reader.readElementText().trimmed());
            CharacterLoader loader(equipment_db, random_affixes_db, sim_settings, raid_control, decoder);

            pchar->get_stats()->get_equipment()->change_setup(i);
            pchar->get_talents()->set_current_index(i);
            pchar->get_enabled_buffs()->get_general_buffs()->change_setup(i);

            loader.initialize_existing(pchar);
        }
    }

    pchar->get_stats()->get_equipment()->change_setup(0);
    pchar->get_talents()->set_current_index(0);
    pchar->get_enabled_buffs()->get_general_buffs()->change_setup(0);

    return pchar;
}

Character* GUIControl::get_new_character(const QString& class_name) {
    raid_controls[class_name] = new RaidControl(sim_settings);
    raid_control = raid_controls[class_name];

    if (class_name == "Druid")
        return new Druid(races["Night Elf"], equipment_db, sim_settings, raid_control);
    if (class_name == "Hunter")
        return new Hunter(races["Dwarf"], equipment_db, sim_settings, raid_control);
    if (class_name == "Mage")
        return new Mage(races["Gnome"], equipment_db, sim_settings, raid_control);
    if (class_name == "Paladin")
        return new Paladin(races["Human"], equipment_db, sim_settings, raid_control);
    if (class_name == "Priest")
        return new Priest(races["Undead"], equipment_db, sim_settings, raid_control);
    if (class_name == "Rogue")
        return new Rogue(races["Troll"], equipment_db, sim_settings, raid_control);
    if (class_name == "Shaman")
        return new Shaman(races["Tauren"], equipment_db, sim_settings, raid_control);
    if (class_name == "Warlock")
        return new Warlock(races["Orc"], equipment_db, sim_settings, raid_control);
    if (class_name == "Warrior")
        return new Warrior(races["Orc"], equipment_db, sim_settings, raid_control);

    check(false, QString("Unknown class '%1'").arg(class_name).toStdString());
    return nullptr;
}

void GUIControl::save_settings() {
    save_gui_settings();

    for (auto& pchar : chars) {
        if (!supported_classes.contains(pchar->class_name))
            continue;

        set_character(pchar);
        save_user_setup(pchar);
    }
}

void GUIControl::save_user_setup(Character* pchar) {
    if (pchar == nullptr)
        pchar = current_char;

    QFile file(QString("Saves/%1-setup.xml").arg(pchar->class_name));
    file.remove();

    if (file.open(QIODevice::ReadWrite)) {
        QXmlStreamWriter stream(&file);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();
        stream.writeStartElement("setups");

        for (int i = 0; i < 3; ++i) {
            pchar->get_stats()->get_equipment()->change_setup(i);
            pchar->get_talents()->set_current_index(i);
            pchar->get_enabled_buffs()->get_general_buffs()->change_setup(i);

            stream.writeTextElement("setup_string", CharacterEncoder(pchar).get_current_setup_string());
        }

        stream.writeEndElement();
        stream.writeEndDocument();
        file.close();
    }
}

void GUIControl::save_gui_settings() {
    QFile file("Saves/GUI-setup.xml");
    file.remove();

    if (file.open(QIODevice::ReadWrite)) {
        QXmlStreamWriter stream(&file);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();
        stream.writeStartElement("settings");

        stream.writeTextElement("class", current_char->class_name);
        stream.writeTextElement("race", current_char->get_race()->get_name());
        stream.writeTextElement("window", active_window);
        stream.writeTextElement("num_iterations_quick_sim", QString("%1").arg(sim_settings->get_combat_iterations_quick_sim()));
        stream.writeTextElement("num_iterations_full_sim", QString("%1").arg(sim_settings->get_combat_iterations_full_sim()));
        stream.writeTextElement("combat_length", QString("%1").arg(sim_settings->get_combat_length()));
        stream.writeTextElement("phase", QString::number(static_cast<int>(sim_settings->get_phase())));
        stream.writeTextElement("ruleset", QString("%1").arg(sim_settings->get_ruleset()));
        stream.writeTextElement("target_creature_type", current_char->get_target()->get_creature_type_string());
        stream.writeTextElement("threads", QString("%1").arg(sim_settings->get_num_threads_current()));

        QSet<SimOption::Name> options = sim_settings->get_active_options();
        for (const auto& option : options)
            stream.writeTextElement("sim_option", QString("%1").arg(option));

        stream.writeEndElement();
        stream.writeEndDocument();
        file.close();
    }
}

void GUIControl::load_gui_settings() {
    QFile file("Saves/GUI-setup.xml");

    if (file.open(QIODevice::ReadOnly)) {
        QXmlStreamReader reader(&file);
        reader.readNextStartElement();

        while (reader.readNextStartElement())
            activate_gui_setting(reader.name(), reader.readElementText().trimmed());
    }

    equipment_db->set_content_phase(sim_settings->get_phase());

    if (current_char == nullptr)
        set_character(chars["Warrior"]);

    if (!current_char->get_faction()->get_faction_races().contains(current_char->get_race()->get_name())) {
        QString name = current_char->get_race()->get_name();
        selectFaction(current_char->get_faction()->is_alliance() ? AvailableFactions::Name::Horde : AvailableFactions::Name::Alliance);
        selectRace(name);
    }
}

void GUIControl::activate_gui_setting(const QStringRef& name, const QString& value) {
    if (name == "class")
        set_character(chars[value]);
    else if (name == "race")
        selectRace(value);
    else if (name == "window")
        active_window = value;
    else if (name == "num_iterations_quick_sim")
        sim_settings->set_combat_iterations_quick_sim(value.toInt());
    else if (name == "num_iterations_full_sim")
        sim_settings->set_combat_iterations_full_sim(value.toInt());
    else if (name == "combat_length")
        sim_settings->set_combat_length(value.toInt());
    else if (name == "phase")
        sim_settings->set_phase(Content::get_phase(value.toInt()));
    else if (name == "ruleset")
        sim_settings->use_ruleset(static_cast<Ruleset>(value.toInt()), current_char);
    else if (name == "target_creature_type" && current_char != nullptr)
        current_char->get_target()->set_creature_type(value);
    else if (name == "threads")
        sim_settings->set_num_threads(value.toInt());
    else if (name == "sim_option")
        sim_settings->add_sim_option(static_cast<SimOption::Name>(value.toInt()));
}

QVariantList GUIControl::get_tooltip_from_item(Item* item) {
    if (item == nullptr)
        return QVariantList();

    QString boe_string = item->get_value("boe") == "yes" ? "Binds when equipped" : "Binds when picked up";
    QString unique = item->get_value("unique") == "yes" ? "Unique" : "";

    QString slot = item->get_value("slot");
    QString dmg_range = "";
    QString weapon_speed = "";
    QString dps = "";

    if (slot == "1H")
        set_weapon_tooltip(item, slot, "One-hand", dmg_range, weapon_speed, dps);
    else if (slot == "MH")
        set_weapon_tooltip(item, slot, "Main Hand", dmg_range, weapon_speed, dps);
    else if (slot == "OH")
      if (item->get_item_type() != WeaponTypes::SHIELD)
        set_weapon_tooltip(item, slot, "Offhand", dmg_range, weapon_speed, dps);
      else
        slot = "Offhand";
    else if (slot == "2H")
        set_weapon_tooltip(item, slot, "Two-hand", dmg_range, weapon_speed, dps);
    else if (slot == "RANGED") {
        const QSet<QString> ranged_weapon_classes = {"Hunter", "Warrior", "Rogue", "Mage", "Warlock", "Priest"};
        if (ranged_weapon_classes.contains(current_char->class_name))
            set_weapon_tooltip(item, slot, "Ranged", dmg_range, weapon_speed, dps);
    } else if (slot == "PROJECTILE")
        set_projectile_tooltip(item, slot, dps);
    else if (slot == "RING")
        slot = "Finger";
    else if (slot == "GLOVES")
        slot = "Hands";
    else if (slot == "BELT")
        slot = "Waist";
    else if (slot == "BOOTS")
        slot = "Feet";
    else if (slot == "SHOULDERS")
        slot = "Shoulder";
    else
        slot = get_initial_upper_case_rest_lower_case(slot);

    QString class_restriction = "";
    set_class_restriction_tooltip(item, class_restriction);

    QString lvl_req = QString("Requires level %1").arg(item->get_value("req_lvl"));

    QVariantList tooltip_info = {QVariant(item->name),
                                 QVariant(item->get_value("quality")),
                                 QVariant(boe_string),
                                 QVariant(unique),
                                 QVariant(slot),
                                 QVariant(get_initial_upper_case_rest_lower_case(item->get_value("type"))),
                                 QVariant(dmg_range),
                                 QVariant(weapon_speed),
                                 QVariant(dps),
                                 QVariant(item->get_base_stat_tooltip()),
                                 QVariant(class_restriction),
                                 QVariant(lvl_req),
                                 QVariant(item->get_equip_effect_tooltip()),
                                 QVariant(item->get_value("flavour_text"))};

    set_set_bonus_tooltip(item, tooltip_info);

    return tooltip_info;
}

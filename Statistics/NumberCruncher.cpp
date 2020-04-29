#include "NumberCruncher.h"

#include <cmath>

#include "ClassStatistics.h"
#include "StatisticsBuff.h"
#include "StatisticsEngine.h"
#include "StatisticsProc.h"
#include "StatisticsResource.h"
#include "StatisticsRotationExecutor.h"
#include "StatisticsSpell.h"
#include "Utils/Check.h"

NumberCruncher::~NumberCruncher() {
    reset();
}

void NumberCruncher::reset() {
    QMutexLocker lock(&mutex);

    for (const auto& class_stat_for_sim_option : class_stats) {
        for (const auto& class_stats_element : class_stat_for_sim_option)
            delete class_stats_element;
    }

    class_stats.clear();
    time_in_combat = 0;
    player_results.clear();
}

void NumberCruncher::add_class_statistic(SimOption::Name key, ClassStatistics* cstat) {
    QMutexLocker lock(&mutex);
    if (!class_stats.contains(key))
        class_stats.insert(key, QVector<ClassStatistics*>({}));

    class_stats[key].append(cstat);

    time_in_combat += cstat->combat_length * cstat->combat_iterations;
    if (!cstat->ignore_non_buff_statistics)
        merge_player_results(cstat);
}

void NumberCruncher::merge_spell_stats(QList<StatisticsSpell*>& vec) {
    QMutexLocker lock(&mutex);

    check(class_stats.contains(SimOption::Name::NoScale), "Missing baseline NoScale statistics");

    long long int total_damage_dealt = 0;
    long long int total_threat_dealt = 0;
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        if (!cstats->ignore_non_buff_statistics) {
            total_damage_dealt += cstats->get_total_personal_damage_dealt();
            total_threat_dealt += cstats->get_total_personal_threat_dealt();
        }
    }

    QSet<QString> handled_entries;
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        if (cstats->ignore_non_buff_statistics)
            continue;

        QMap<QString, StatisticsSpell*>::const_iterator it = cstats->spell_statistics.constBegin();
        auto end = cstats->spell_statistics.constEnd();
        while (it != end) {
            if (handled_entries.contains(it.key())) {
                ++it;
                continue;
            }
            handled_entries.insert(it.key());
            merge_spell_entry(it.key(), it.value()->get_icon(), total_damage_dealt, total_threat_dealt, vec);
            ++it;
        }
    }
}

void NumberCruncher::merge_spell_entry(
    const QString& name, const QString& icon, long long total_damage_dealt, long long total_threat_dealt, QList<StatisticsSpell*>& vec) {
    auto result = new StatisticsSpell(name, icon);
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        if (!cstats->spell_statistics.contains(name))
            continue;

        result->add(cstats->spell_statistics[name]);
    }

    result->set_percentage_of_damage_dealt(total_damage_dealt);
    result->set_percentage_of_threat_dealt(total_threat_dealt);
    vec.append(result);
}

void NumberCruncher::merge_buff_stats(QList<StatisticsBuff*>& vec, const bool include_debuffs) {
    QMutexLocker lock(&mutex);

    check(class_stats.contains(SimOption::Name::NoScale), "Missing baseline NoScale statistics");

    QSet<QString> handled_entries;
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        QMap<QString, StatisticsBuff*>::const_iterator it = cstats->buff_statistics.constBegin();
        auto end = cstats->buff_statistics.constEnd();
        while (it != end) {
            if (handled_entries.contains(it.key()) || (it.value()->is_debuff() != include_debuffs)) {
                ++it;
                continue;
            }
            handled_entries.insert(it.key());
            merge_buff_entry(it.key(), it.value()->get_icon(), vec);
            ++it;
        }
    }
}

void NumberCruncher::merge_buff_entry(const QString& name, const QString& icon, QList<StatisticsBuff*>& vec) {
    auto result = new StatisticsBuff(name, icon, false);
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        if (!cstats->buff_statistics.contains(name))
            continue;

        result->add(cstats->buff_statistics[name]);
    }

    if (result->get_avg_uptime() < 0.0000001)
        return;

    vec.append(result);
}

void NumberCruncher::merge_proc_stats(QList<StatisticsProc*>& vec) {
    QMutexLocker lock(&mutex);

    check(class_stats.contains(SimOption::Name::NoScale), "Missing baseline NoScale statistics");

    QSet<QString> handled_entries;
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        if (cstats->ignore_non_buff_statistics)
            continue;

        QMap<QString, StatisticsProc*>::const_iterator it = cstats->proc_statistics.constBegin();
        auto end = cstats->proc_statistics.constEnd();
        while (it != end) {
            if (handled_entries.contains(it.key())) {
                ++it;
                continue;
            }
            handled_entries.insert(it.key());
            merge_proc_entry(it.key(), it.value()->get_icon(), vec);
            ++it;
        }
    }
}

void NumberCruncher::merge_proc_entry(const QString& name, const QString& icon, QList<StatisticsProc*>& vec) {
    auto result = new StatisticsProc(name, icon, time_in_combat);
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        if (!cstats->proc_statistics.contains(name))
            continue;

        result->add(cstats->proc_statistics[name]);
    }

    vec.append(result);
}

void NumberCruncher::merge_resource_stats(QList<StatisticsResource*>& vec) {
    QMutexLocker lock(&mutex);

    check(class_stats.contains(SimOption::Name::NoScale), "Missing baseline NoScale statistics");

    QSet<QString> handled_entries;
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        if (cstats->ignore_non_buff_statistics)
            continue;

        QMap<QString, StatisticsResource*>::const_iterator it = cstats->resource_statistics.constBegin();
        auto end = cstats->resource_statistics.constEnd();
        while (it != end) {
            if (handled_entries.contains(it.key())) {
                ++it;
                continue;
            }
            handled_entries.insert(it.key());
            merge_resource_entry(it.key(), it.value()->get_icon(), vec);
            ++it;
        }
    }
}

void NumberCruncher::merge_resource_entry(const QString& name, const QString& icon, QList<StatisticsResource*>& vec) {
    auto result = new StatisticsResource(name, icon, time_in_combat);
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        if (!cstats->resource_statistics.contains(name))
            continue;

        result->add(cstats->resource_statistics[name]);
    }

    vec.append(result);
}

void NumberCruncher::merge_engine_stats(StatisticsEngine* statistics_engine) {
    for (const auto& cstats : class_stats[SimOption::Name::NoScale]) {
        if (!cstats->ignore_non_buff_statistics)
            statistics_engine->add(cstats->get_engine_statistics());
    }
}

void NumberCruncher::merge_rotation_executor_stats(QList<QList<ExecutorOutcome*>>& list) {
    if (class_stats[SimOption::Name::NoScale].empty())
        return;

    QList<StatisticsRotationExecutor*> merge_base = class_stats[SimOption::Name::NoScale][0]->rotation_executor_statistics;

    for (int i = 1; i < class_stats[SimOption::Name::NoScale].size(); ++i) {
        auto cstat = class_stats[SimOption::Name::NoScale][i];
        if (cstat->ignore_non_buff_statistics)
            continue;

        check((merge_base.size() == cstat->rotation_executor_statistics.size()), "Mismatch ClassStatistics rotation executor size");

        for (int j = 0; j < cstat->rotation_executor_statistics.size(); ++j)
            merge_base[j]->add(cstat->rotation_executor_statistics[j]);
    }

    for (const auto& rotation_executor : merge_base)
        list.append(rotation_executor->get_list_of_executor_outcomes());
}

void NumberCruncher::calculate_stat_weights(QList<ScaleResult*>& list) {
    QMutexLocker lock(&mutex);

    QMap<SimOption::Name, double> dps_per_option;

    QMap<SimOption::Name, QVector<ClassStatistics*>>::const_iterator it = class_stats.constBegin();
    auto end = class_stats.constEnd();
    while (it != end) {
        dps_per_option.insert(it.key(), get_dps_for_option(it.key()));
        ++it;
    }

    check(dps_per_option.contains(SimOption::Name::NoScale), "Missing baseline NoScale statistics");

    double base_value = dps_per_option.take(SimOption::Name::NoScale);

    QMap<SimOption::Name, double>::const_iterator dps_it = dps_per_option.constBegin();
    while (dps_it != dps_per_option.constEnd()) {
        double absolute_diff = dps_it.value() - base_value;
        double relative_diff = absolute_diff / base_value;

        double standard_deviation = get_standard_deviation_for_option(dps_it.key());
        double confidence_interval = get_confidence_interval_for_option(dps_it.key(), standard_deviation);

        list.append(new ScaleResult(dps_it.key(), 0.0, 0.0, absolute_diff, relative_diff, standard_deviation, confidence_interval));
        ++dps_it;
    }
}

double NumberCruncher::get_personal_dps(SimOption::Name option) const {
    return get_dps_for_option(option);
}

double NumberCruncher::get_personal_tps(SimOption::Name option) const {
    return get_tps_for_option(option);
}

double NumberCruncher::get_raid_dps() const {
    double sum = 0.0;
    for (const auto& result : player_results)
        sum += result->dps;

    return sum;
}

double NumberCruncher::get_raid_tps() const {
    double sum = 0.0;
    for (const auto& result : player_results)
        sum += result->tps;

    return sum;
}

QPair<double, double> NumberCruncher::get_min_max_dps_for_option(SimOption::Name option) const {
    check(class_stats.contains(option), "Missing option for requested calculation");

    double min_dps = std::numeric_limits<double>::max();
    double max_dps = std::numeric_limits<double>::min();

    for (const auto& class_stat : class_stats[option]) {
        for (const auto dps : class_stat->dps_for_iterations) {
            if (dps < min_dps)
                min_dps = dps;

            if (dps > max_dps)
                max_dps = dps;
        }
    }

    return {min_dps, max_dps};
}

double NumberCruncher::get_dps_for_option(SimOption::Name option) const {
    check(class_stats.contains(option), "Missing option for requested calculation");

    QVector<double> dps;

    for (const auto& class_stat : class_stats[option]) {
        if (!class_stat->ignore_non_buff_statistics)
            dps.append(class_stat->get_personal_result()->dps);
    }

    double dps_sum = 0;
    for (const auto& value : dps)
        dps_sum += value;

    dps_sum /= dps.size();

    return dps_sum;
}

double NumberCruncher::get_tps_for_option(SimOption::Name option) const {
    check(class_stats.contains(option), "Missing option for requested calculation");

    QVector<double> tps;

    for (const auto& class_stat : class_stats[option]) {
        if (!class_stat->ignore_non_buff_statistics)
            tps.append(class_stat->get_personal_result()->tps);
    }

    double tps_sum = 0;
    for (const auto& value : tps)
        tps_sum += value;

    tps_sum /= tps.size();

    return tps_sum;
}

ScaleResult* NumberCruncher::get_dps_distribution() const {
    double standard_deviation = get_standard_deviation_for_option(SimOption::Name::NoScale);
    double confidence_interval = get_confidence_interval_for_option(SimOption::Name::NoScale, standard_deviation);
    QPair<double, double> dps = get_min_max_dps_for_option(SimOption::Name::NoScale);

    return new ScaleResult(SimOption::Name::NoScale, dps.first, dps.second, 0.0, 0.0, standard_deviation, confidence_interval);
}

void NumberCruncher::merge_player_results(ClassStatistics* cstat) {
    check(!cstat->player_results.empty(), "NumberCruncher expected non-empty ClassStatistics::player_results");

    if (player_results.empty()) {
        player_results = cstat->player_results;
        collected_iterations = cstat->player_results[0]->iterations;
        return;
    }

    check((cstat->player_results.size() == player_results.size()), "NumberCruncher::merge_raid_results - Mismatch in expected result sizes");
    const int iterations = cstat->player_results[0]->iterations;
    collected_iterations += iterations;
    const double ratio_new_results = static_cast<double>(iterations) / collected_iterations;
    const double ratio_previous_results = 1.0 - ratio_new_results;

    for (int i = 0; i < cstat->player_results.size(); ++i) {
        check((player_results[i]->player_name == cstat->player_results[i]->player_name), "Mismatch in player names");
        check((iterations == cstat->player_results[i]->iterations), "Mismatch in iterations between players of same raid");

        player_results[i]->dps = player_results[i]->dps * ratio_previous_results + cstat->player_results[i]->dps * ratio_new_results;
        player_results[i]->iterations += cstat->player_results[i]->iterations;
    }
}

double NumberCruncher::get_standard_deviation_for_option(SimOption::Name option) const {
    check(class_stats.contains(option), "Missing option for requested calculation");

    double mean = get_dps_for_option(option);
    double variance = 0;
    int counter = 0;
    for (const auto& class_stat : class_stats[option]) {
        if (class_stat->ignore_non_buff_statistics)
            continue;

        for (const auto& dps : class_stat->dps_for_iterations) {
            ++counter;
            variance = variance + (std::pow(dps - mean, 2) - variance) / counter;
        }
    }

    return sqrt(variance);
}

double NumberCruncher::get_confidence_interval_for_option(SimOption::Name option, const double standard_deviation) const {
    check(class_stats.contains(option), "Missing option for requested calculation");

    double z_value = 1.960;

    int population = 0;
    for (const auto& class_stat : class_stats[option]) {
        if (class_stat->ignore_non_buff_statistics)
            continue;

        population += class_stat->dps_for_iterations.size();
    }

    return z_value * (standard_deviation / std::sqrt(population));
}

QString get_name_for_option(const SimOption::Name option) {
    if (option == SimOption::Name::ScaleAgility)
        return "+10 Agility";
    if (option == SimOption::Name::ScaleStrength)
        return "+10 Strength";
    if (option == SimOption::Name::ScaleHitChance)
        return "+1% Hit Chance";
    if (option == SimOption::Name::ScaleCritChance)
        return "+1% Crit Chance";
    if (option == SimOption::Name::ScaleAttackPower)
        return "+10 Attack Power";
    if (option == SimOption::Name::ScaleAxeSkill)
        return "+1 Axe Skill";
    if (option == SimOption::Name::ScaleDaggerSkill)
        return "+1 Dagger Skill";
    if (option == SimOption::Name::ScaleMaceSkill)
        return "+1 Mace Skill";
    if (option == SimOption::Name::ScaleSwordSkill)
        return "+1 Sword Skill";
    if (option == SimOption::Name::ScaleIntellect)
        return "+10 Intellect";
    if (option == SimOption::Name::ScaleSpirit)
        return "+10 Spirit";
    if (option == SimOption::Name::ScaleMp5)
        return "+10 Mana per 5";
    if (option == SimOption::Name::ScaleSpellDamage)
        return "+10 Spell Damage";
    if (option == SimOption::Name::ScaleSpellCritChance)
        return "+1% Spell Crit Chance";
    if (option == SimOption::Name::ScaleSpellHitChance)
        return "+1% Spell Hit Chance";
    if (option == SimOption::Name::ScaleSpellPenetration)
        return "+10 Spell Penetration";

    return "<unset scale name>";
}

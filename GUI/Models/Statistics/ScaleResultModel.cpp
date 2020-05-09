#include "ScaleResultModel.h"

#include "NumberCruncher.h"
#include "SimOption.h"
#include "SortDirection.h"
#include "StatisticsProc.h"

bool option(ScaleResult* lhs, ScaleResult* rhs) {
    return get_name_for_option(lhs->option) < get_name_for_option(rhs->option);
}

bool absolute_value(ScaleResult* lhs, ScaleResult* rhs) {
    return lhs->absolute_value > rhs->absolute_value;
}

bool relative_value(ScaleResult* lhs, ScaleResult* rhs) {
    return lhs->relative_value > rhs->relative_value;
}

bool standard_deviation(ScaleResult* lhs, ScaleResult* rhs) {
    return lhs->standard_deviation > rhs->standard_deviation;
}

bool confidence_interval(ScaleResult* lhs, ScaleResult* rhs) {
    return lhs->confidence_interval > rhs->confidence_interval;
}

ScaleResultModel::ScaleResultModel(NumberCruncher* statistics_source, bool for_dps, QObject* parent) :
    QAbstractListModel(parent), statistics_source(statistics_source), for_dps{for_dps} {
    this->current_sorting_method = ScaleResultSorting::Methods::ByAbsoluteValue;
    this->sorting_methods.insert(ScaleResultSorting::Methods::ByName, SortDirection::Forward);
    this->sorting_methods.insert(ScaleResultSorting::Methods::ByAbsoluteValue, SortDirection::Forward);
    this->sorting_methods.insert(ScaleResultSorting::Methods::ByRelativeValue, SortDirection::Forward);
}

ScaleResultModel::~ScaleResultModel() {
    for (const auto& i : scale_results)
        delete i;
}

void ScaleResultModel::selectSort(const int method) {
    emit layoutAboutToBeChanged();

    auto sorting_method = static_cast<ScaleResultSorting::Methods>(method);
    switch (sorting_method) {
    case ScaleResultSorting::Methods::ByName:
        std::sort(scale_results.begin(), scale_results.end(), option);
        select_new_method(sorting_method);
        break;
    case ScaleResultSorting::Methods::ByAbsoluteValue:
        std::sort(scale_results.begin(), scale_results.end(), absolute_value);
        select_new_method(sorting_method);
        break;
    case ScaleResultSorting::Methods::ByRelativeValue:
        std::sort(scale_results.begin(), scale_results.end(), relative_value);
        select_new_method(sorting_method);
        break;
    case ScaleResultSorting::Methods::ByStandardDeviation:
        std::sort(scale_results.begin(), scale_results.end(), standard_deviation);
        select_new_method(sorting_method);
        break;
    case ScaleResultSorting::Methods::ByConfidenceInterval:
        std::sort(scale_results.begin(), scale_results.end(), confidence_interval);
        select_new_method(sorting_method);
        break;
    }

    emit layoutChanged();
}

void ScaleResultModel::select_new_method(const ScaleResultSorting::Methods new_method) {
    if (sorting_methods[new_method] == SortDirection::Reverse)
        std::reverse(scale_results.begin(), scale_results.end());

    const auto next_sort_direction = sorting_methods[new_method] == SortDirection::Forward ? SortDirection::Reverse : SortDirection::Forward;
    current_sorting_method = new_method;

    for (auto& direction : sorting_methods)
        direction = SortDirection::Forward;

    sorting_methods[current_sorting_method] = next_sort_direction;

    emit sortingMethodChanged();
}

int ScaleResultModel::get_current_sorting_method() const {
    return static_cast<int>(current_sorting_method);
}

void ScaleResultModel::update_statistics() {
    if (!scale_results.empty()) {
        beginResetModel();

        for (const auto& i : scale_results)
            delete i;

        scale_results.clear();
        endResetModel();
    }

    beginInsertRows(QModelIndex(), 0, rowCount());
    if (for_dps)
      statistics_source->calculate_stat_weights_for_dps(scale_results);
    else
      statistics_source->calculate_stat_weights_for_tps(scale_results);

    endInsertRows();

    emit layoutAboutToBeChanged();
    std::sort(scale_results.begin(), scale_results.end(), absolute_value);
    emit layoutChanged();
}

int ScaleResultModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return scale_results.count();
}

QVariant ScaleResultModel::data(const QModelIndex& index, int role) const {
    if (index.row() < 0 || index.row() >= scale_results.count())
        return QVariant();

    const ScaleResult* scale_result = scale_results[index.row()];

    if (role == ScaleResultSorting::ByName)
        return get_name_for_option(scale_result->option);
    if (role == ScaleResultSorting::ByAbsoluteValue)
        return QString::number(scale_result->absolute_value, 'f', 2);
    if (role == ScaleResultSorting::ByRelativeValue)
        return QString::number(scale_result->relative_value * 100, 'f', 2);
    if (role == ScaleResultSorting::ByStandardDeviation)
        return QString::number(scale_result->standard_deviation, 'f', 2);
    if (role == ScaleResultSorting::ByConfidenceInterval)
        return QString::number(scale_result->confidence_interval, 'f', 2);

    return QVariant();
}

QHash<int, QByteArray> ScaleResultModel::roleNames() const {
    QHash<int, QByteArray> roles;
    roles[ScaleResultSorting::ByName] = "_name";
    roles[ScaleResultSorting::ByAbsoluteValue] = "_absvalue";
    roles[ScaleResultSorting::ByRelativeValue] = "_relvalue";
    roles[ScaleResultSorting::ByStandardDeviation] = "_standarddev";
    roles[ScaleResultSorting::ByConfidenceInterval] = "_confidenceinterval";

    return roles;
}

#include "ActiveItemStatFilterModel.h"

#include "Character.h"
#include "EquipmentDb.h"
#include "Item.h"
#include "ItemModel.h"
#include "Utils/Check.h"
#include "WeaponModel.h"

bool ItemStatFilter::item_passes_filter(const Item* item) const {
    unsigned item_stat_value = item->get_stat_value_via_flag(this->item_stat_flag);

    switch (comparator) {
    case StatComparator::Less:
        return item_stat_value < cmp_value;
    case StatComparator::LEQ:
        return item_stat_value <= cmp_value;
    case StatComparator::Equal:
        return item_stat_value == cmp_value;
    case StatComparator::Greater:
        return item_stat_value > cmp_value;
    case StatComparator::GEQ:
        return item_stat_value >= cmp_value;
    default:
        check(false, "Unknown stat comparator");
        return false;
    }
}

QString ItemStatFilter::get_comparator_string() const {
    switch (comparator) {
    case StatComparator::Less:
        return "<";
    case StatComparator::LEQ:
        return "<=";
    case StatComparator::Equal:
        return "==";
    case StatComparator::Greater:
        return ">";
    case StatComparator::GEQ:
        return ">=";
    default:
        check(false, "Unknown stat comparator");
        return "";
    }
}

ActiveItemStatFilterModel::ActiveItemStatFilterModel(QObject* parent) : QAbstractListModel(parent) {
    this->item_model = nullptr;
    this->weapon_model = nullptr;
}

ActiveItemStatFilterModel::~ActiveItemStatFilterModel() {
    for (const auto& filter : active_item_stat_filters) {
        delete filter;
    }
}

bool ActiveItemStatFilterModel::item_passes_active_stat_filters(const Item* item) const {
    for (const auto& filter : active_item_stat_filters) {
        if (!filter->item_passes_filter(item))
            return false;
    }

    return true;
}

void ActiveItemStatFilterModel::update_affected_models() {
    this->item_model->update_items();
    this->weapon_model->update_items();
}

void ActiveItemStatFilterModel::set_item_model(ItemModel* item_model) {
    this->item_model = item_model;
}

void ActiveItemStatFilterModel::set_weapon_model(WeaponModel* weapon_model) {
    this->weapon_model = weapon_model;
}

void ActiveItemStatFilterModel::add_filter(const ItemStats item_stat_flag, const QString& description) {
    for (const auto& filter : active_item_stat_filters) {
        if (filter->item_stat_flag == item_stat_flag)
            return;
    }

    beginInsertRows(QModelIndex(), rowCount(), rowCount());
    active_item_stat_filters.append(new ItemStatFilter(item_stat_flag, description));
    endInsertRows();
    update_affected_models();
}

void ActiveItemStatFilterModel::removeFilter(const int item_stat_flag_int) {
    auto item_stat_flag = static_cast<ItemStats>(item_stat_flag_int);
    for (int i = 0; i < active_item_stat_filters.size(); ++i) {
        if (active_item_stat_filters[i]->item_stat_flag == item_stat_flag) {
            emit layoutAboutToBeChanged();
            active_item_stat_filters.removeAt(i);
            emit layoutChanged();
            update_affected_models();
            return;
        }
    }
}

void ActiveItemStatFilterModel::clearFilters() {
    emit layoutAboutToBeChanged();
    for (const auto& filter : active_item_stat_filters) {
        delete filter;
    }

    active_item_stat_filters.clear();
    emit layoutChanged();

    update_affected_models();
}

void ActiveItemStatFilterModel::changeComparator(const unsigned item_stat_flag_unsigned, const unsigned comparator) {
    if (comparator > StatComparator::GEQ)
        return;

    auto item_stat_flag = static_cast<ItemStats>(item_stat_flag_unsigned);
    for (const auto& active_item_stat_filter : active_item_stat_filters) {
        if (active_item_stat_filter->item_stat_flag == item_stat_flag) {
            emit layoutAboutToBeChanged();
            active_item_stat_filter->comparator = comparator;
            emit layoutChanged();
            update_affected_models();
            return;
        }
    }
}

void ActiveItemStatFilterModel::changeCompareValue(const unsigned item_stat_flag_unsigned, const unsigned cmp_value) {
    auto item_stat_flag = static_cast<ItemStats>(item_stat_flag_unsigned);
    for (const auto& active_item_stat_filter : active_item_stat_filters) {
        if (active_item_stat_filter->item_stat_flag == item_stat_flag) {
            emit layoutAboutToBeChanged();
            active_item_stat_filter->cmp_value = cmp_value;
            emit layoutChanged();
            update_affected_models();
            return;
        }
    }
}

int ActiveItemStatFilterModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return active_item_stat_filters.count();
}

QVariant ActiveItemStatFilterModel::data(const QModelIndex& index, int role) const {
    if (index.row() < 0 || index.row() >= active_item_stat_filters.count())
        return QVariant();

    if (role == ItemStatRole)
        return static_cast<int>(active_item_stat_filters[index.row()]->item_stat_flag);
    if (role == DescriptionRole)
        return active_item_stat_filters[index.row()]->description;
    if (role == ComparatorRole)
        return active_item_stat_filters[index.row()]->get_comparator_string();
    if (role == CompareValueRole)
        return active_item_stat_filters[index.row()]->cmp_value;

    return QVariant();
}

QHash<int, QByteArray> ActiveItemStatFilterModel::roleNames() const {
    QHash<int, QByteArray> roles;

    roles[ItemStatRole] = "_itemstatflag";
    roles[DescriptionRole] = "_description";
    roles[ComparatorRole] = "_comparator";
    roles[CompareValueRole] = "_comparevalue";

    return roles;
}

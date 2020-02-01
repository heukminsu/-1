#pragma once

#include <QVector>

class Character;
class Item;

class CharacterEncoder {
public:
    CharacterEncoder(Character* pchar = nullptr);

    void set_character(Character* pchar);
    QString get_current_setup_string();

protected:
private:
    Character* pchar;

    QString pchar_str;
    void new_element();
    void new_list_element();
    void add_vector_values_only(const QString& name, const QVector<QString>& vec);
    void add_vector(QVector<QPair<QString, QVector<QPair<QString, QString>>>> &vec);
    void add_item(const QString& key, Item* item);
    void key_val(const QString& key, const QString& value);
    void key_val_list(const QString& key, const QString& value);

    void add_enchants();
    void add_affixes();
};

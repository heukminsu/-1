#pragma once

#include <QObject>
#include <QString>

class Content : public QObject {
    Q_OBJECT

public:
    enum class Phase : int
    {
        MoltenCore = 1,
        DireMaul,
        BlackwingLair,
        ZulGurub,
        AhnQiraj,
        Naxxramas,
    };

    Q_ENUM(Phase)

    static QString get_description_for_phase(const Content::Phase phase);
    static QString get_shortname_for_phase(const Content::Phase phase);
    static Phase get_phase(const int);
};

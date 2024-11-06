// parameterdefinition.h
#ifndef PARAMETERDEFINITION_H
#define PARAMETERDEFINITION_H

#include <QString>
#include <QMetaType>
#include <QVector>
#include <QList>

struct ConversionDefinition {
    QString units;
    QString expr;
    QString format;
    QString gauge_min;
    QString gauge_max;
    QString gauge_step;
};

struct ParameterDefinition {
    QString category;
    QString id;
    QString name;
    QString description;
    QString address;
    QString units;
    QString expression;
    QString format;
    QVector<QPair<QString, QString>> unitsList;
    float min;
    float max;
    float step;
    quint32 value;
    bool selected = false;

    QList<ConversionDefinition> conversions;
};

Q_DECLARE_METATYPE(ParameterDefinition)

#endif // PARAMETERDEFINITION_H

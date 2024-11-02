// parameterdefinition.h
#ifndef PARAMETERDEFINITION_H
#define PARAMETERDEFINITION_H

#include <QString>
#include <QMetaType>
#include <QVector>

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
};

Q_DECLARE_METATYPE(ParameterDefinition)  // Добавьте эту строку для использования в QVariant

#endif // PARAMETERDEFINITION_H

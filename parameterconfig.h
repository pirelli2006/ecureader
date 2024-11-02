#ifndef PARAMETERCONFIG_H
#define PARAMETERCONFIG_H

#include <QString>

struct ParameterConfig {
    QString name;
    int address;
    int length;
    QString unit;
    QString description;
    QString formula;
};

#endif // PARAMETERCONFIG_H

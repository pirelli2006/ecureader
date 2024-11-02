#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QString>
#include <QVector>
#include <QMap>
#include <QXmlStreamReader>

class ConfigManager
{
public:
    static ConfigManager& instance();

    struct Conversion {
        QString units;
        QString expr;
        QString format;
        double gauge_min = 0;
        double gauge_max = 0;
        double gauge_step = 0;
        QString storagetype;
        double evaluate(const std::vector<double>& values) const;
    };

    struct Parameter {
        QString id;
        QString name;
        QString desc;
        QString units;
        uint32_t address = 0;
        uint8_t bit = 0;
        uint8_t ecubyteindex = 0;
        uint8_t target = 0;
        QString storagetype;
        QVector<Conversion> conversions;
    };

    bool loadConfiguration(const QString& filename);
    const Parameter* getParameter(const QString& id) const;
    QVector<Parameter> getAllParameters() const;

private:
    ConfigManager() = default;

    bool parseParameter(QXmlStreamReader& xml);
    bool parseConversion(QXmlStreamReader& xml, Parameter& param);

    QMap<QString, Parameter> m_parameters;
};

#endif // CONFIGMANAGER_H

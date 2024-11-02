// loggerdefinitionloader.h
#ifndef LOGGERDEFINITIONLOADER_H
#define LOGGERDEFINITIONLOADER_H

#include <QString>
#include <QMap>
#include <QList>
#include <QXmlStreamReader>
#include "parameterdefinition.h"

class LoggerDefinitionLoader
{
public:
    LoggerDefinitionLoader();
    bool loadFromXml(const QString& filePath);
    QMap<QString, QList<ParameterDefinition>> getParameters() const;

private:
    QMap<QString, QList<ParameterDefinition>> m_parameters;
    void parseProtocol(QXmlStreamReader& xml);
    void parseParameter(QXmlStreamReader& xml, const QString& category);
};

#endif // LOGGERDEFINITIONLOADER_H

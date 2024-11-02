#include "loggerdefinitionloader.h"
#include <QFile>
#include <QXmlStreamReader>
#include <QDebug>

LoggerDefinitionLoader::LoggerDefinitionLoader() {}

bool LoggerDefinitionLoader::loadFromXml(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open file:" << filePath;
        return false;
    }

    QXmlStreamReader xml(&file);
    while (!xml.atEnd() && !xml.hasError()) {
        QXmlStreamReader::TokenType token = xml.readNext();
        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "logger") {
                while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "logger")) {
                    if (xml.tokenType() == QXmlStreamReader::StartElement) {
                        if (xml.name() == "protocol") {
                            parseProtocol(xml);
                        }
                    }
                    xml.readNext();
                }
            }
        }
    }

    file.close();
    if (xml.hasError()) {
        qDebug() << "XML error:" << xml.errorString();
        return false;
    }

    return true;
}

void LoggerDefinitionLoader::parseProtocol(QXmlStreamReader& xml)
{
    QString protocolId = xml.attributes().value("id").toString();
    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "protocol")) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "parameter") {
                parseParameter(xml, protocolId);
            }
        }
        xml.readNext();
    }
}

void LoggerDefinitionLoader::parseParameter(QXmlStreamReader& xml, const QString& category)
{
    ParameterDefinition param;
    param.category = category;
    param.id = xml.attributes().value("id").toString();
    param.name = xml.attributes().value("name").toString();
    param.description = xml.attributes().value("desc").toString();

    while (!(xml.tokenType() == QXmlStreamReader::EndElement && xml.name() == "parameter")) {
        if (xml.tokenType() == QXmlStreamReader::StartElement) {
            if (xml.name() == "address") {
                param.address = xml.readElementText();
            } else if (xml.name() == "conversion") {
                param.units = xml.attributes().value("units").toString();
                param.expression = xml.attributes().value("expr").toString();
                param.format = xml.attributes().value("format").toString();
                param.min = xml.attributes().value("gauge_min").toString().toFloat();
                param.max = xml.attributes().value("gauge_max").toString().toFloat();
                param.step = xml.attributes().value("gauge_step").toString().toFloat();
            }
        }
        xml.readNext();
    }

    m_parameters[category].append(param);
}

QMap<QString, QList<ParameterDefinition>> LoggerDefinitionLoader::getParameters() const
{
    return m_parameters;
}

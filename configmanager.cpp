#include "configmanager.h"
#include <QFile>
#include <QDebug>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::loadConfiguration(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open config file:" << filename;
        return false;
    }

    QXmlStreamReader xml(&file);
    m_parameters.clear();

    while (!xml.atEnd() && !xml.hasError()) {
        auto token = xml.readNext();

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "parameter") {
                if (!parseParameter(xml)) {
                    qWarning() << "Failed to parse parameter at line" << xml.lineNumber();
                    return false;
                }
            }
        }
    }

    if (xml.hasError()) {
        qWarning() << "XML parsing error:" << xml.errorString();
        return false;
    }

    return true;
}

bool ConfigManager::parseParameter(QXmlStreamReader& xml) {
    Parameter param;
    auto attrs = xml.attributes();

    param.id = attrs.value("id").toString();
    param.name = attrs.value("name").toString();
    param.desc = attrs.value("desc").toString();
    param.ecubyteindex = attrs.value("ecubyteindex").toUInt();
    param.target = attrs.value("target").toUInt();
    param.storagetype = attrs.value("storagetype").toString();

    while (!xml.atEnd()) {
        auto token = xml.readNext();

        if (token == QXmlStreamReader::EndElement && xml.name() == "parameter") {
            break;
        }

        if (token == QXmlStreamReader::StartElement) {
            if (xml.name() == "address") {
                bool ok;
                param.address = xml.readElementText().toUInt(&ok, 16);
                if (!ok) return false;
            }
            else if (xml.name() == "bit") {
                bool ok;
                param.bit = xml.readElementText().toUInt(&ok);
                if (!ok) return false;
            }
            else if (xml.name() == "conversion") {
                if (!parseConversion(xml, param)) {
                    return false;
                }
            }
        }
    }

    m_parameters[param.id] = param;
    return true;
}

bool ConfigManager::parseConversion(QXmlStreamReader& xml, Parameter& param) {
    Conversion conv;
    auto attrs = xml.attributes();

    conv.units = attrs.value("units").toString();
    conv.expr = attrs.value("expr").toString();
    conv.format = attrs.value("format").toString();
    conv.storagetype = attrs.value("storagetype").toString();

    bool ok;
    if (attrs.hasAttribute("gauge_min")) {
        conv.gauge_min = attrs.value("gauge_min").toDouble(&ok);
        if (!ok) return false;
    }
    if (attrs.hasAttribute("gauge_max")) {
        conv.gauge_max = attrs.value("gauge_max").toDouble(&ok);
        if (!ok) return false;
    }
    if (attrs.hasAttribute("gauge_step")) {
        conv.gauge_step = attrs.value("gauge_step").toDouble(&ok);
        if (!ok) return false;
    }

    param.conversions.append(conv);
    return true;
}

const ConfigManager::Parameter* ConfigManager::getParameter(const QString& id) const {
    auto it = m_parameters.find(id);
    return it != m_parameters.end() ? &it.value() : nullptr;
}

QVector<ConfigManager::Parameter> ConfigManager::getAllParameters() const {
    return m_parameters.values().toVector();
}

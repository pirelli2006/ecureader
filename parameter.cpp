// parameter.cpp
#include "parameter.h"
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>

Parameter::Parameter() = default;

Parameter::Parameter(const QString& name, const QString& id, const QString& description, const QString& units)
    : m_name(name), m_id(id), m_description(description), m_units(units)
{
}

Parameter Parameter::fromJson(const QJsonObject& json) {
    Parameter param;
    param.setName(json["name"].toString());
    param.setId(json["id"].toString());
    param.address = json["address"].toString();
    param.startBit = json["startBit"].toInt();
    param.bitLength = json["bitLength"].toInt();
    param.setDescription(json["description"].toString());

    if (json.contains("conversions")) {
        QJsonArray convArray = json["conversions"].toArray();
        for (const QJsonValue& convValue : convArray) {
            QJsonObject convObj = convValue.toObject();

            param.conversions.push_back(std::make_unique<Expression>(
                convObj["expression"].toString(),
                convObj["units"].toString(),
                convObj["format"].toString(),
                convObj["min"].toDouble(),
                convObj["max"].toDouble(),
                convObj["step"].toDouble()
                ));

            Conversion conv;
            conv.name = convObj["name"].toString();
            conv.formula = convObj["expression"].toString();
            conv.units = convObj["units"].toString();
            param.addConversion(conv);
        }
    }

    return param;
}

void Parameter::addConversion(const Conversion& conversion)
{
    m_conversions.push_back(conversion);
}

void Parameter::setConversions(const QVector<Conversion>& conversions)
{
    m_conversions = conversions;
}

Parameter::Parameter(const Parameter& other)
    : address(other.address)
    , startBit(other.startBit)
    , bitLength(other.bitLength)
    , m_name(other.m_name)
    , m_id(other.m_id)
    , m_description(other.m_description)
    , m_units(other.m_units)
    , m_conversions(other.m_conversions)
{
    for (const auto& expr : other.conversions) {
        conversions.push_back(expr->clone());
    }
}



Parameter::Parameter(Parameter&& other) noexcept
    : address(std::move(other.address)), startBit(other.startBit), bitLength(other.bitLength),
    conversions(std::move(other.conversions)),
    m_name(std::move(other.m_name)), m_id(std::move(other.m_id)),
    m_description(std::move(other.m_description)), m_units(std::move(other.m_units)),
    m_conversions(std::move(other.m_conversions))
{
}

Parameter& Parameter::operator=(const Parameter& other)
{
    if (this != &other) {
        address = other.address;
        startBit = other.startBit;
        bitLength = other.bitLength;
        m_name = other.m_name;
        m_id = other.m_id;
        m_description = other.m_description;
        m_units = other.m_units;
        m_conversions = other.m_conversions;

        conversions.clear();
        for (const auto& expr : other.conversions) {
            conversions.push_back(expr->clone());
        }
    }
    return *this;
}

Parameter& Parameter::operator=(Parameter&& other) noexcept
{
    if (this != &other) {
        address = std::move(other.address);
        startBit = other.startBit;
        bitLength = other.bitLength;
        conversions = std::move(other.conversions);
        m_name = std::move(other.m_name);
        m_id = std::move(other.m_id);
        m_description = std::move(other.m_description);
        m_units = std::move(other.m_units);
        m_conversions = std::move(other.m_conversions);
    }
    return *this;
}

#include "parameter.h"

Parameter::Parameter()
{
}

Parameter::Parameter(const QString& name, const QString& id, const QString& description, const QString& units)
    : m_name(name)
    , m_id(id)
    , m_description(description)
    , m_units(units)
{
}

void Parameter::addConversion(const Conversion& conversion)
{
    m_conversions.append(conversion);
}

void Parameter::setConversions(const QVector<Conversion>& conversions)
{
    m_conversions = conversions;
}

QString Parameter::getDescription() const
{
    return m_description;
}

QString Parameter::getUnits() const
{
    return m_units;
}

void Parameter::setUnits(const QString& units)
{
    m_units = units;
}

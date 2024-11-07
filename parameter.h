#ifndef PARAMETER_H
#define PARAMETER_H

#include <QString>
#include <QVector>
#include <QJsonObject>
#include <memory>
#include "expression.h"
#include <vector>

struct Conversion {
    QString name;
    QString formula;
    QString units;
};

class Parameter
{
public:
    Parameter();
    Parameter(const QString& name, const QString& id, const QString& description, const QString& units, double currentValue = 0.0);
    ~Parameter();
    std::vector<std::unique_ptr<Expression>> conversions;

    // Используем правило пяти
    Parameter(const Parameter& other);
    Parameter(Parameter&& other) noexcept;
    Parameter& operator=(const Parameter& other);
    Parameter& operator=(Parameter&& other) noexcept;

    // Геттеры
    double getMinValue() const { return minValue; }
    double getMaxValue() const { return maxValue; }
    double getInitialValue() const { return initialValue; }
    QString getName() const { return m_name; }
    QString getId() const { return m_id; }
    QString getDescription() const { return m_description; }
    QString getUnits() const { return m_units; }
    const QVector<Conversion>& getConversions() const { return m_conversions; }
    double getCurrentValue() const { return m_currentValue; } // Добавленный геттер

    // Сеттеры
    void setName(const QString& name) { m_name = name; }
    void setId(const QString& id) { m_id = id; }
    void setDescription(const QString& description) { m_description = description; }
    void setUnits(const QString& units) { m_units = units; }
    void setCurrentValue(double currentValue) { m_currentValue = currentValue; } // Добавленный сеттер
    void addConversion(const Conversion& conversion);
    void setConversions(const QVector<Conversion>& conversions);

    // Публичные члены для прямого доступа
    QString address;
    int startBit;
    int bitLength;

    // Создание из JSON
    static Parameter fromJson(const QJsonObject& json);

    friend QDebug operator<<(QDebug dbg, const Parameter& param) {
        dbg << "Parameter("
            << "Name:" << param.getName()
            << ", Units:" << param.getUnits()
            << ", Min:" << param.getMinValue()
            << ", Max:" << param.getMaxValue()
            << ", Initial Value:" << param.getInitialValue()
            << ")";
        return dbg;
    }

private:
    QString m_name;
    QString m_id;
    QString m_description;
    QString m_units;
    QVector<Conversion> m_conversions;  // Простые конверсии
    double m_currentValue; // Поле для хранения текущего значения
    QString name;
    QString units;
    double minValue;
    double maxValue;
    double initialValue;
};

#endif // PARAMETER_H

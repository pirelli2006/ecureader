#ifndef PARAMETER_H
#define PARAMETER_H

#include <QString>
#include <QVector>

struct Conversion {
    QString name;
    QString formula;
    QString units;
};

class Parameter
{
public:
    Parameter();
    Parameter(const QString& name, const QString& id, const QString& description, const QString& units);

    QString getDescription() const;
    QString getUnits() const;
    void setUnits(const QString& units);


    QString getName() const { return m_name; }
    QString getId() const { return m_id; }

    void setName(const QString& name) { m_name = name; }
    void setId(const QString& id) { m_id = id; }

    // Добавляем методы для работы с конверсиями
    void addConversion(const Conversion& conversion);
    void setConversions(const QVector<Conversion>& conversions);
    QVector<Conversion> getConversions() const { return m_conversions; }

private:
    QString m_name;
    QString m_id;
    QVector<Conversion> m_conversions; // Добавляем член для хранения конверсий
    QString m_description;
    QString m_units;
};

#endif // PARAMETER_H

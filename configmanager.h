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
        QString units; // Единицы измерения
        QString expr;  // Выражение для преобразования
        QString format; // Формат вывода
        double gauge_min = 0; // Минимальное значение для шкалы
        double gauge_max = 0; // Максимальное значение для шкалы
        double gauge_step = 0; // Шаг для шкалы
        QString storagetype; // Тип хранения данных

        double evaluate(const std::vector<double>& values) const; // Оценка значения
    };

    struct Parameter {
        QString id; // Идентификатор параметра
        QString name; // Название параметра
        QString desc; // Описание параметра
        QString units; // Единицы измерения
        uint32_t address = 0; // Адрес параметра
        uint8_t bit = 0; // Бит параметра
        uint8_t ecubyteindex = 0; // Индекс
        uint8_t target = 0; // Целевой индекс
        QString storagetype; // Тип хранения данных
        QVector<Conversion> conversions; // Преобразования для параметра
    };

    bool loadConfiguration(const QString& filename); // Загрузка конфигурации из файла
    const Parameter* getParameter(const QString& id) const; // Получение параметра по идентификатору
    QVector<Parameter> getAllParameters() const; // Получение всех параметров

private:
    ConfigManager() = default;

    bool parseParameter(QXmlStreamReader& xml); // Парсинг параметра
    bool parseConversion(QXmlStreamReader& xml, Parameter& param); // Парсинг преобразования

    QMap<QString, Parameter> m_parameters; // Хранение параметров
};

#endif // CONFIGMANAGER_H

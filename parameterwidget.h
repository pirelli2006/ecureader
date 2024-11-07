#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>

class ParameterWidget : public QWidget
{
    Q_OBJECT
public:
    enum WindowSize {
        SMALL,
        MEDIUM
    };
    explicit ParameterWidget(const QString &name,
                             const QString& units,
                             double minValue,
                             double maxValue,
                             double initialValue,
                             QWidget *parent = nullptr);

    void updateValue(const QString &value, const QString& units);
    void setWindowSize(WindowSize size);

signals:
    // Сигналы для оповещения об изменении состояния
    void warnStateChanged(bool isChecked);
    void warnThresholdChanged(double newThreshold);
    void parameterSelected(const QString& parameterName);

private:
    QLabel *valueLabel;
    QLabel *unitsLabel;
    QLabel *maxValueLabel;
    QLabel *minValueLabel;
    QCheckBox *warnCheckBox;
    QComboBox *warnComboBox;
    QLineEdit *warnValueEdit;
    WindowSize currentSize; // Текущий размер окна

    // Функция для обновления стилей в зависимости от размера окна
    void updateStyles();
};

#endif // PARAMETERWIDGET_H

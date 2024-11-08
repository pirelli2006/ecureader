#ifndef PARAMETERGRAPHICSWIDGET_H
#define PARAMETERGRAPHICSWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QMap>
#include <QSize>

enum class WindowStyleType {
    Minimal,
    Standard,
    Full
};

struct StyleConfig {
    QSize size;
    QString valueLabelStyle;
};

class ParameterGraphicsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ParameterGraphicsWidget(const QString& name,
                                     const QString& units,
                                     double minValue,
                                     double maxValue,
                                     double initialValue,
                                     QWidget* parent = nullptr);

    void updateValue(double value);
    void setMinMax(double minValue, double maxValue);
    void applyStyle(WindowStyleType styleType);
    double getMinValue() const
    {
        return minValue;
    }

    // Определение метода для получения максимального значения
    double getMaxValue() const
    {
        return maxValue;
    }


private:
    QString parameterName;
    QString parameterUnits;
    double minValue;
    double maxValue;
    double initialValue;

    QLabel* titleLabel;
    QLabel* valueLabel;
    QLabel* maxLabel;
    QLabel* maxValueLabel;
    QLabel* minLabel;
    QLabel* minValueLabel;
    QCheckBox* warnCheckBox;
    QComboBox* warnComboBox;
    QLineEdit* warnValueEdit;
};

#endif // PARAMETERGRAPHICSWIDGET_H

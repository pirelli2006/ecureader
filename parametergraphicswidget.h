#ifndef PARAMETERGRAPHICSWIDGET_H
#define PARAMETERGRAPHICSWIDGET_H

#include <QGraphicsWidget>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>

class ParameterGraphicsWidget : public QGraphicsWidget
{
    Q_OBJECT
public:
    explicit ParameterGraphicsWidget(const QString& name,
                                     const QString& units,
                                     double minValue,
                                     double maxValue,
                                     double initialValue,
                                     QGraphicsItem* parent = nullptr);

    void updateValue(double value);
    double getMinValue() const { return minValueLabel->text().toDouble(); }
    double getMaxValue() const { return maxValueLabel->text().toDouble(); }
    void setMinMax(double minValue, double maxValue);

signals:
    void warnStateChanged(bool isChecked);
    void warnThresholdChanged(double newThreshold);

private:
    QLabel* titleLabel;
    QLabel* valueLabel;
    QLabel* maxLabel;
    QLabel* maxValueLabel;
    QLabel* minLabel;
    QLabel* minValueLabel;
    QCheckBox* warnCheckBox;
    QComboBox* warnComboBox;
    QLineEdit* warnValueEdit;

    double minValue; // Объявляем переменные-члены
    double maxValue; // Объявляем переменные-члены
};

#endif // PARAMETERGRAPHICSWIDGET_H

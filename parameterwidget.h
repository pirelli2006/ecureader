// parameterwidget.h
#ifndef PARAMETERWIDGET_H
#define PARAMETERWIDGET_H

#include <QWidget>
#include <QLabel>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFrame>

class ParameterWidget : public QFrame {
    Q_OBJECT
public:
    explicit ParameterWidget(const QString& name, const QString& units,
                             double minValue, double maxValue, QWidget *parent = nullptr);

    void setValue(double value);
    bool isEnabled() const { return m_checkBox->isChecked(); }

private slots:
    void checkCondition();
    void startBlinking();
    void stopBlinking();

private:
    QLabel *m_nameLabel;
    QLabel *m_valueLabel;
    QLabel *m_unitsLabel;
    QLabel *m_minMaxLabel;
    QCheckBox *m_checkBox;
    QComboBox *m_conditionCombo;
    QLineEdit *m_thresholdEdit;

    double m_currentValue;
    double m_minValue;
    double m_maxValue;

    QTimer *m_blinkTimer;
    bool m_isBlinking;

    void setupUI();
    void createConnections();
    void updateStyle(bool alert);
};

#endif // PARAMETERWIDGET_H

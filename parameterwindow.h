#ifndef PARAMETERWINDOW_H
#define PARAMETERWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>

class ParameterWindow : public QWidget
{
    Q_OBJECT

public:
    ParameterWindow(const QString& paramName, QWidget* parent = nullptr);

public slots:
    void updateValue(const QString& paramName, const QString& value, const QString& units);

private:
    QLabel* m_nameLabel;
    QLabel* m_valueLabel;
};

#endif // PARAMETERWINDOW_H

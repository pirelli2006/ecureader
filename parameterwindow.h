#ifndef PARAMETERWINDOW_H
#define PARAMETERWINDOW_H

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

class ParameterWindow : public QWidget
{


public:
    Q_OBJECT

   ParameterWindow(const QString& paramName, QWidget* parent = nullptr);

public slots:
    void updateValue(const QString& value, const QString& units);

private:
    QHBoxLayout* m_layout;
    QLabel* m_nameLabel;
    QLabel* m_valueLabel;
};

#endif // PARAMETERWINDOW_H

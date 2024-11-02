#include "ParameterWindow.h"

ParameterWindow::ParameterWindow(const QString& paramName, QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(paramName);
    setMinimumSize(200, 100);

    QVBoxLayout* layout = new QVBoxLayout(this);
    m_nameLabel = new QLabel(paramName, this);
    m_valueLabel = new QLabel("--", this);
    m_valueLabel->setAlignment(Qt::AlignCenter);
    QFont valueFont = m_valueLabel->font();
    valueFont.setPointSize(valueFont.pointSize() * 2);
    m_valueLabel->setFont(valueFont);

    layout->addWidget(m_nameLabel);
    layout->addWidget(m_valueLabel);
}

void ParameterWindow::updateValue(const QString& paramName, const QString& value, const QString& units)
{
    if (paramName == windowTitle()) {
        m_valueLabel->setText(value + " " + units);
    }
}

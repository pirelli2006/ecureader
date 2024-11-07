#include "ParameterWindow.h"
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>

ParameterWindow::ParameterWindow(const QString& paramName, QWidget* parent)
    : QWidget(parent)
{
    setWindowTitle(paramName);
    setMinimumSize(30, 30);

    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_layout = new QHBoxLayout(this);
    m_nameLabel = new QLabel(paramName, this);
    m_valueLabel = new QLabel("--", this);
    m_valueLabel->setAlignment(Qt::AlignCenter);
    QFont valueFont = m_valueLabel->font();
    valueFont.setPointSize(valueFont.pointSize() * 2);
    m_valueLabel->setFont(valueFont);

    // Добавляем виджеты в m_layout
    m_layout->addWidget(m_nameLabel);
    m_layout->addWidget(m_valueLabel);
}

void ParameterWindow::updateValue(const QString& value, const QString& units)
{
        m_valueLabel->setText(value + " " + units);
}

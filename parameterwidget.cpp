#include "parameterwidget.h"
#include <QTimer>

ParameterWidget::ParameterWidget(const QString& name, const QString& units,
                                 double minValue, double maxValue, QWidget *parent)
    : QFrame(parent)
    , m_currentValue(0)
    , m_minValue(minValue)
    , m_maxValue(maxValue)
    , m_isBlinking(false)
{
    setFrameStyle(QFrame::Box | QFrame::Raised);
    setLineWidth(2);

    setupUI();
    createConnections();

    m_nameLabel->setText(name);
    m_unitsLabel->setText(units);
    m_minMaxLabel->setText(QString("Min: %1 Max: %2").arg(minValue).arg(maxValue));

    m_blinkTimer = new QTimer(this);
    m_blinkTimer->setInterval(500); // 500ms blink interval
    connect(m_blinkTimer, &QTimer::timeout, this, [this]() {
        updateStyle(m_isBlinking);
        m_isBlinking = !m_isBlinking;
    });
}

void ParameterWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Верхняя строка: название и значение
    QHBoxLayout *topLayout = new QHBoxLayout();
    m_nameLabel = new QLabel(this);
    m_valueLabel = new QLabel(this);
    m_unitsLabel = new QLabel(this);
    m_minMaxLabel = new QLabel(this);

    topLayout->addWidget(m_nameLabel);
    topLayout->addWidget(m_valueLabel);
    topLayout->addWidget(m_unitsLabel);
    topLayout->addStretch();
    topLayout->addWidget(m_minMaxLabel);

    // Нижняя строка: настройки условия
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    m_checkBox = new QCheckBox("Enable", this);
    m_conditionCombo = new QComboBox(this);
    m_conditionCombo->addItems(QStringList() << ">" << "<" << "=");
    m_thresholdEdit = new QLineEdit(this);
    m_thresholdEdit->setPlaceholderText("Threshold value");

    bottomLayout->addWidget(m_checkBox);
    bottomLayout->addWidget(m_conditionCombo);
    bottomLayout->addWidget(m_thresholdEdit);

    mainLayout->addLayout(topLayout);
    mainLayout->addLayout(bottomLayout);
}

void ParameterWidget::createConnections()
{
    connect(m_checkBox, &QCheckBox::toggled, this, &ParameterWidget::checkCondition);
    connect(m_conditionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ParameterWidget::checkCondition);
    connect(m_thresholdEdit, &QLineEdit::textChanged,
            this, &ParameterWidget::checkCondition);
}

void ParameterWidget::setValue(double value)
{
    m_currentValue = value;
    m_valueLabel->setText(QString::number(value, 'f', 2));
    checkCondition();
}

void ParameterWidget::checkCondition()
{
    if (!m_checkBox->isChecked()) {
        stopBlinking();
        return;
    }

    bool ok;
    double threshold = m_thresholdEdit->text().toDouble(&ok);
    if (!ok) return;

    bool condition = false;
    switch (m_conditionCombo->currentIndex()) {
    case 0: // >
        condition = m_currentValue > threshold;
        break;
    case 1: // <
        condition = m_currentValue < threshold;
        break;
    case 2: // =
        condition = qFuzzyCompare(m_currentValue, threshold);
        break;
    }

    if (condition) {
        startBlinking();
    } else {
        stopBlinking();
    }
}

void ParameterWidget::startBlinking()
{
    if (!m_blinkTimer->isActive()) {
        m_blinkTimer->start();
    }
}

void ParameterWidget::stopBlinking()
{
    if (m_blinkTimer->isActive()) {
        m_blinkTimer->stop();
    }
    updateStyle(false);
}

void ParameterWidget::updateStyle(bool alert)
{
    if (alert) {
        setStyleSheet("ParameterWidget { border: 2px solid red; }");
    } else {
        setStyleSheet("ParameterWidget { border: 2px solid gray; }");
    }
}

void ParameterWidget::updateValue(double value)
{
    m_valueLabel->setText(QString::number(value, 'f', 2));
}

void ParameterWidget::updateValue(const QString& value)
{
    m_valueLabel->setText(value);
}

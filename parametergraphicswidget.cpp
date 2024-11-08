#include "parametergraphicswidget.h"
#include <QMap>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>

static const QMap<WindowStyleType, StyleConfig> styleConfigs = {
    { WindowStyleType::Minimal, { QSize(200, 100), "color: white; font-size: 20pt; font-weight: bold;" } },
    { WindowStyleType::Standard, { QSize(260, 150), "color: white; font-size: 28pt; font-weight: bold;" } },
    { WindowStyleType::Full, { QSize(320, 200), "color: white; font-size: 36pt; font-weight: bold;" } }
};

ParameterGraphicsWidget::ParameterGraphicsWidget(const QString& name,
                                                 const QString& units,
                                                 double minValue,
                                                 double maxValue,
                                                 double initialValue,
                                                 QWidget* parent)
    : QWidget(parent), parameterName(name), parameterUnits(units), minValue(minValue), maxValue(maxValue), initialValue(initialValue)
{
    // Главный layout
    auto* mainLayout = new QVBoxLayout(this);
    setStyleSheet("background-color: #1c1c1e; border: 1px solid #444; border-radius: 4px;");
    mainLayout->setContentsMargins(8, 8, 8, 8);

    // Заголовок параметра
    titleLabel = new QLabel(QString("%1 (%2)").arg(parameterName, parameterUnits));
    titleLabel->setAlignment(Qt::AlignCenter);
    titleLabel->setStyleSheet("color: #d1d1d6; font-size: 10pt;");
    titleLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    mainLayout->addWidget(titleLabel);

    // Layout для основного значения и max/min значений
    auto* valueLayout = new QHBoxLayout();
    valueLabel = new QLabel(QString::number(initialValue, 'f', 2));
    valueLabel->setAlignment(Qt::AlignCenter);
    valueLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    valueLabel->setStyleSheet("color: white; font-weight: bold;");
    valueLayout->addWidget(valueLabel);

    // Компоновка для max/min значений справа от основного значения
    auto* minMaxLayout = new QVBoxLayout();
    maxLabel = new QLabel("max:");
    maxLabel->setStyleSheet("color: #d1d1d6; font-size: 8pt;");
    maxValueLabel = new QLabel(QString::number(maxValue, 'f', 2));
    maxValueLabel->setStyleSheet("color: #d1d1d6; font-size: 8pt;");
    minMaxLayout->addWidget(maxLabel);
    minMaxLayout->addWidget(maxValueLabel);

    minLabel = new QLabel("min:");
    minLabel->setStyleSheet("color: #d1d1d6; font-size: 8pt;");
    minValueLabel = new QLabel(QString::number(minValue, 'f', 2));
    minValueLabel->setStyleSheet("color: #d1d1d6; font-size: 8pt;");
    minMaxLayout->addWidget(minLabel);
    minMaxLayout->addWidget(minValueLabel);

    valueLayout->addLayout(minMaxLayout);
    mainLayout->addLayout(valueLayout);

    // Layout для предупреждений
    auto* warnLayout = new QHBoxLayout();
    warnCheckBox = new QCheckBox("Warn");
    warnCheckBox->setStyleSheet("color: #d1d1d6; font-size: 8pt;");
    warnComboBox = new QComboBox;
    warnComboBox->addItems({"above", "below"});
    warnComboBox->setStyleSheet("background-color: #2c2c2e; color: white; font-size: 8pt;");
    warnValueEdit = new QLineEdit;
    warnValueEdit->setPlaceholderText("Value");
    warnValueEdit->setStyleSheet("background-color: #2c2c2e; color: white; font-size: 8pt;");
    warnValueEdit->setFixedWidth(100);

    warnLayout->addWidget(warnCheckBox);
    warnLayout->addWidget(warnComboBox);
    warnLayout->addWidget(warnValueEdit);
    mainLayout->addLayout(warnLayout);

    // Устанавливаем начальный стиль
    applyStyle(WindowStyleType::Standard);
}

void ParameterGraphicsWidget::updateValue(double value) {
    if (value < minValue || value > maxValue) {
        valueLabel->setStyleSheet("color: red; font-size: 24pt; font-weight: bold;");
    } else {
        valueLabel->setStyleSheet("color: white; font-size: 24pt; font-weight: bold;");
    }
    valueLabel->setText(QString::number(value, 'f', 2));
}

void ParameterGraphicsWidget::setMinMax(double minValue, double maxValue) {
    this->minValue = minValue;
    this->maxValue = maxValue;
    minValueLabel->setText(QString::number(minValue, 'f', 2));
    maxValueLabel->setText(QString::number(maxValue, 'f', 2));
}

void ParameterGraphicsWidget::applyStyle(WindowStyleType styleType) {
    // Управляем видимостью элементов в зависимости от стиля
    bool showMinMax = (styleType == WindowStyleType::Standard || styleType == WindowStyleType::Full);
    bool showWarnings = (styleType == WindowStyleType::Full);

    // Устанавливаем видимость min/max значений
    maxLabel->setVisible(showMinMax);
    maxValueLabel->setVisible(showMinMax);
    minLabel->setVisible(showMinMax);
    minValueLabel->setVisible(showMinMax);

    // Устанавливаем видимость элементов предупреждения
    warnCheckBox->setVisible(showWarnings);
    warnComboBox->setVisible(showWarnings);
    warnValueEdit->setVisible(showWarnings);

    // Применение размеров и стиля для текущего типа окна
    if (styleConfigs.contains(styleType)) {
        const StyleConfig& config = styleConfigs[styleType];
        setMinimumSize(config.size);
        setMaximumSize(config.size);
        valueLabel->setStyleSheet(config.valueLabelStyle);
    }

    update(); // Обновляем виджет после изменения стиля
}

#include "parameterwidget.h"

ParameterWidget::ParameterWidget(const QString &name,
                                 const QString& units,
                                 double minValue,
                                 double maxValue,
                                 double initialValue,
                                 QWidget *parent) :
    QWidget(parent),
    currentSize(WindowSize::SMALL) // Изначально размер окна - SMALL
{
    // Заголовок
    auto *titleLabel = new QLabel(name, this);
    titleLabel->setStyleSheet("color: white; font-size: 10pt; border-bottom: 1px solid gray;");
    titleLabel->setAlignment(Qt::AlignCenter);

    // Значение
    valueLabel = new QLabel(QString::number(initialValue), this);
    valueLabel->setStyleSheet("color: white; font-size: 24pt; font-weight: bold;");
    valueLabel->setAlignment(Qt::AlignCenter);

    // Единицы измерения
    unitsLabel = new QLabel(units, this);
    unitsLabel->setStyleSheet("color: white; font-size: 10pt;");
    unitsLabel->setAlignment(Qt::AlignCenter);

    // Макс/Мин
    auto *maxLabel = new QLabel("max", this);
    maxLabel->setStyleSheet("color: white; font-size: 8pt; border-bottom: 1px solid gray;");
    maxValueLabel = new QLabel(QString::number(maxValue), this);
    maxValueLabel->setStyleSheet("color: white; font-size: 10pt; border-bottom: 1px solid gray;");

    auto *minLabel = new QLabel("min", this);
    minLabel->setStyleSheet("color: white; font-size: 8pt; border-bottom: 1px solid gray;");
    minValueLabel = new QLabel(QString::number(minValue), this);
    minValueLabel->setStyleSheet("color: white; font-size: 10pt;");

    auto *minmaxLayout = new QVBoxLayout;
    minmaxLayout->addWidget(maxLabel);
    minmaxLayout->addWidget(maxValueLabel);
    minmaxLayout->addSpacing(10);
    minmaxLayout->addWidget(minLabel);
    minmaxLayout->addWidget(minValueLabel);

    auto *valueLayout = new QHBoxLayout;
    valueLayout->addWidget(valueLabel, 1);
    valueLayout->addLayout(minmaxLayout);

    // Чекбокс и выбор
    warnCheckBox = new QCheckBox("Warn", this);
    warnCheckBox->setStyleSheet("color: white;");
    connect(warnCheckBox, &QCheckBox::stateChanged, this, &ParameterWidget::warnStateChanged);

    warnComboBox = new QComboBox(this);
    warnComboBox->addItem("above");
    warnComboBox->setStyleSheet("background-color: #4a5568; color: white; border: 1px solid gray;");

    warnValueEdit = new QLineEdit(this);
    warnValueEdit->setStyleSheet("background-color: black; color: white; border: 1px solid gray;");
    warnValueEdit->setFixedWidth(60);

    auto *warnLayout = new QHBoxLayout;
    warnLayout->addWidget(warnCheckBox);
    warnLayout->addWidget(warnComboBox);
    warnLayout->addWidget(warnValueEdit);

    // Общий layout
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(titleLabel);
    mainLayout->addLayout(valueLayout);
    mainLayout->addWidget(unitsLabel); // Добавляем единицы измерения
    mainLayout->addLayout(warnLayout);
    mainLayout->setContentsMargins(10, 10, 10, 10);

    // Стилизация виджета
    setStyleSheet("background-color: black;");
}

void ParameterWidget::updateValue(const QString &value, const QString& units) {
    valueLabel->setText(value);
    unitsLabel->setText(units); // Обновляем единицы измерения
}

void ParameterWidget::setWindowSize(WindowSize size) {
    currentSize = size;
    updateStyles();
}

void ParameterWidget::updateStyles() {
    if (currentSize == SMALL) {
        // Применяем стили для маленького размера
        setFixedSize(150, 100);
    } else {
        // Применяем стили для среднего размера
        setFixedSize(200, 150);
    }
}

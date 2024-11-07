#include "parametergraphicswidget.h"
#include <QGraphicsLinearLayout>
#include <QGraphicsProxyWidget>
#include <QLabel>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QWidget>
#include <QFrame>

ParameterGraphicsWidget::ParameterGraphicsWidget(const QString& name,
                                                 const QString& units,
                                                 double minValue,
                                                 double maxValue,
                                                 double initialValue,
                                                 QGraphicsItem* parent)
    : QGraphicsWidget(parent), minValue(minValue), maxValue(maxValue)
{
    // Создаем контейнер QWidget
    QWidget *containerWidget = new QWidget();
    containerWidget->setStyleSheet("background-color: black; border: 1px solid #6b7280; border-radius: 5px;");

    // Установка компоновки для контейнера
    auto* mainLayout = new QVBoxLayout(containerWidget);

    // --- Создание виджетов ---
    titleLabel = new QLabel(QString("<b>%1 (%2)</b>").arg(name, units));
    titleLabel->setStyleSheet("color: white; font-size: 10pt; padding-bottom: 2px;");
    titleLabel->setAlignment(Qt::AlignCenter);

    valueLabel = new QLabel(QString::number(initialValue, 'f', 2));
    valueLabel->setStyleSheet("color: white; font-size: 24pt; font-weight: bold;");
    valueLabel->setAlignment(Qt::AlignCenter);

    maxLabel = new QLabel("max:");
    maxLabel->setStyleSheet("color: white; font-size: 8pt;");
    maxValueLabel = new QLabel(QString::number(maxValue, 'f', 2));
    maxValueLabel->setFixedSize(90, 30);
    maxValueLabel->setStyleSheet("color: white; font-size: 10pt;");

    minLabel = new QLabel("min:");
    minLabel->setStyleSheet("color: white; font-size: 8pt;");
    minValueLabel = new QLabel(QString::number(minValue, 'f', 2));
    minValueLabel->setFixedSize(90, 30);
    minValueLabel->setStyleSheet("color: white; font-size: 10pt;");

    warnCheckBox = new QCheckBox("Warn");
    warnCheckBox->setStyleSheet("color: white; font-size: 8pt;");
    warnCheckBox->setFixedSize(90, 30);
    connect(warnCheckBox, &QCheckBox::stateChanged, this, &ParameterGraphicsWidget::warnStateChanged);

    warnComboBox = new QComboBox;
    warnComboBox->addItems({"above", "below"});
    warnComboBox->setStyleSheet("background-color: #4a5568; color: white; font-size: 8pt; border: 1px solid #6b7280; padding: 2px;");

    warnValueEdit = new QLineEdit;
    warnValueEdit->setStyleSheet("background-color: black; color: white; font-size: 8pt; border: 1px solid #6b7280; padding: 2px;");
    warnValueEdit->setPlaceholderText("Value");
    connect(warnValueEdit, &QLineEdit::editingFinished, this, [this]() {
        bool ok;
        double newThreshold = warnValueEdit->text().toDouble(&ok);
        if (ok) {
            emit warnThresholdChanged(newThreshold);
        } else {
            warnValueEdit->setStyleSheet("background-color: red;");
        }
    });

    // --- Установка компоновки ---
    // Добавляем заголовок
    mainLayout->addWidget(titleLabel);

    // Создаем горизонтальный layout для valueLabel, разделителя и блока min/max
    auto* valueMinMaxLayout = new QHBoxLayout();
    mainLayout->addLayout(valueMinMaxLayout);

    // Добавляем valueLabel
    valueMinMaxLayout->addWidget(valueLabel);

    // Добавляем вертикальный разделитель
    QFrame* separator = new QFrame();
    separator->setFrameShape(QFrame::VLine);
    separator->setStyleSheet("background-color: #6b7280; width: 1px;"); // Устанавливаем цвет и ширину
    //valueMinMaxLayout->addWidget(separator); // Добавляем разделитель в layout

    // Добавляем отступы перед разделителем
    valueMinMaxLayout->addSpacing(5); // Добавляем отступ перед разделителем
    valueMinMaxLayout->addWidget(separator); // Добавляем разделитель в layout
    valueMinMaxLayout->addSpacing(5); // Добавляем отступ после разделителя


    // Создаем вертикальный layout для min/max
    auto* minMaxLayout = new QVBoxLayout();
    minMaxLayout->setSpacing(5); // Увеличиваем расстояние между элементами
    minMaxLayout->setContentsMargins(0, 0, 5, 0); // Устанавливаем отступы (left, top, right, bottom)
    valueMinMaxLayout->addLayout(minMaxLayout);

    // Добавляем метки и значения max/min
    minMaxLayout->addWidget(maxLabel);
    minMaxLayout->addWidget(maxValueLabel);
    minMaxLayout->addSpacing(10); // Добавляем отступ между max и min
    minMaxLayout->addWidget(minLabel);
    minMaxLayout->addWidget(minValueLabel);



    // Создаем warnLayout
    auto* warnLayout = new QHBoxLayout();
    mainLayout->addLayout(warnLayout);

    // Добавляем warnCheckBox
    warnLayout->addWidget(warnCheckBox);

    // Добавляем warnComboBox
    warnLayout->addWidget(warnComboBox);

    // Добавляем warnValueEdit
    warnLayout->addWidget(warnValueEdit);

    // Устанавливаем основной layout для QGraphicsWidget
    auto* graphicsLayout = new QGraphicsLinearLayout(Qt::Vertical, this);
    setLayout(graphicsLayout);

    // Создаем QGraphicsProxyWidget для контейнера
    auto* proxyContainer = new QGraphicsProxyWidget(this);
    proxyContainer->setWidget(containerWidget);

    // Добавляем proxyContainer в графический layout
    graphicsLayout->addItem(proxyContainer);
}

// Метод для обновления значения
void ParameterGraphicsWidget::updateValue(double value) {
    if (value < minValue || value > maxValue) {
        // Изменение цвета текста на красный для индикации ошибки
        valueLabel->setStyleSheet("color: red; font-size: 24pt; font-weight: bold;");
    } else {
        valueLabel->setStyleSheet("color: white; font-size: 24pt; font-weight: bold;");
    }
    valueLabel->setText(QString::number(value, 'f', 2));
}

// Метод для установки новых значений min и max
void ParameterGraphicsWidget::setMinMax(double minValue, double maxValue) {
    this->minValue = minValue; // Сохраняем новое значение
    this->maxValue = maxValue; // Сохраняем новое значение
    minValueLabel->setText(QString::number(minValue, 'f', 2));
    maxValueLabel->setText(QString::number(maxValue, 'f', 2));
}

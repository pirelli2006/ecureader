#include "parameterselectiondialog.h"
#include "parameter.h"
#include "configmanager.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QTreeWidget>
#include <QDialogButtonBox>

ParameterSelectionDialog::ParameterSelectionDialog(const QVector<ConfigManager::Parameter>& parameters, QWidget *parent)
    : QDialog(parent)
{
    setupUi();

    for (const auto& param : parameters) {
        addParameterToTree(param);
    }

    connect(m_selectAllButton, &QPushButton::clicked, this, &ParameterSelectionDialog::selectAll);
    connect(m_deselectAllButton, &QPushButton::clicked, this, &ParameterSelectionDialog::deselectAll);
}

ParameterSelectionDialog::~ParameterSelectionDialog() = default;

void ParameterSelectionDialog::setupUi()
{
    setWindowTitle(tr("Select Parameters"));
    setMinimumWidth(400);

    auto* mainLayout = new QVBoxLayout(this);

    auto* buttonLayout = new QHBoxLayout;
    m_selectAllButton = new QPushButton(tr("Select All"), this);
    m_deselectAllButton = new QPushButton(tr("Deselect All"), this);
    buttonLayout->addWidget(m_selectAllButton);
    buttonLayout->addWidget(m_deselectAllButton);
    buttonLayout->addStretch();
    mainLayout->addLayout(buttonLayout);

    m_treeWidget = new QTreeWidget(this);
    m_treeWidget->setHeaderLabels({tr("Parameter"), tr("Description"), tr("Units")});
    m_treeWidget->setRootIsDecorated(false);
    m_treeWidget->setAlternatingRowColors(true);
    mainLayout->addWidget(m_treeWidget);

    m_buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(m_buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(m_buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(m_buttonBox);
}

void ParameterSelectionDialog::addParameterToTree(const ConfigManager::Parameter& param)
{
    auto* item = new QTreeWidgetItem(m_treeWidget);
    item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
    item->setCheckState(0, Qt::Unchecked);

    // Используем прямой доступ к полям вместо методов
    item->setText(0, param.name);
    item->setText(1, param.desc);
    item->setText(2, param.units);

    // Предполагая, что id тоже является прямым полем
    item->setData(0, Qt::UserRole, param.id);
}

QStringList ParameterSelectionDialog::getSelectedParameters() const
{
    QStringList selected;
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        if ((*it)->checkState(0) == Qt::Checked) {
            selected << (*it)->data(0, Qt::UserRole).toString();
        }
        ++it;
    }
    return selected;
}

void ParameterSelectionDialog::selectAll()
{
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        (*it)->setCheckState(0, Qt::Checked);
        ++it;
    }
}

void ParameterSelectionDialog::deselectAll()
{
    QTreeWidgetItemIterator it(m_treeWidget);
    while (*it) {
        (*it)->setCheckState(0, Qt::Unchecked);
        ++it;
    }
}

void ParameterSelectionDialog::updateConversionsList(const Parameter& parameter)
{
    // Этот метод может быть реализован, если нужна функциональность конверсий
}

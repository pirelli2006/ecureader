#ifndef PARAMETERSELECTIONDIALOG_H
#define PARAMETERSELECTIONDIALOG_H

#include <QDialog>
#include <QList>
#include "configmanager.h"

class QPushButton;
class QTreeWidget;
class QDialogButtonBox;
class Parameter;

class ParameterSelectionDialog : public QDialog
{
    Q_OBJECT

public:
    using Parameter = ConfigManager::Parameter;
    explicit ParameterSelectionDialog(const QVector<ConfigManager::Parameter>& parameters, QWidget *parent = nullptr);
    ~ParameterSelectionDialog();

    QStringList getSelectedParameters() const;

private slots:
    void selectAll();
    void deselectAll();
    void updateConversionsList(const Parameter& parameter);

private:
    void setupUi();
    void addParameterToTree(const ConfigManager::Parameter& param);
    QVector<ConfigManager::Parameter> m_parameters;

    QPushButton* m_selectAllButton;
    QPushButton* m_deselectAllButton;
    QTreeWidget* m_treeWidget;
    QDialogButtonBox* m_buttonBox;
};

#endif // PARAMETERSELECTIONDIALOG_H

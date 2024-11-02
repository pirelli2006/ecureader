// preferencesdialog.h
#ifndef PREFERENCESDIALOG_H
#define PREFERENCESDIALOG_H

#include <QDialog>

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit PreferencesDialog(QWidget *parent = nullptr);
    ~PreferencesDialog();

    // Добавьте здесь публичные методы для получения/установки настроек

private:
         // Приватные члены и методы
};

#endif // PREFERENCESDIALOG_H

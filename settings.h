#ifndef SETTINGS_H
#define SETTINGS_H

#include <QString>
#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QByteArray>

class Settings {
public:
    static Settings& getInstance() {
        static Settings instance;
        return instance;
    }

    void init() {
        QString settingsPath = QDir::homePath() + "/.ecureader";
        QDir dir(settingsPath);
        if (!dir.exists()) {
            if (!dir.mkpath(".")) {
                qWarning() << "Failed to create settings directory:" << settingsPath;
            }
        }
        if (settings) {
            delete settings;
        }
        settings = new QSettings(settingsPath + "/settings.ini", QSettings::IniFormat);

        // Проверка доступности файла настроек
        if (!QFile::exists(settings->fileName())) {
            qWarning() << "Settings file does not exist:" << settings->fileName();
        } else {
            qDebug() << "Settings file found:" << settings->fileName();
            qDebug() << "Settings file is writable:" << QFileInfo(settings->fileName()).isWritable();
        }
    }

    // Методы для основного окна
    void setSelectedAdapter(const QString& adapter) {
        settings->setValue("MainWindow/adapter", adapter);
        settings->sync();
    }

    QString getSelectedAdapter() const {
        return settings->value("MainWindow/adapter", "").toString();
    }

    void setMainWindowGeometry(const QByteArray& geometry) {
        settings->setValue("MainWindow/geometry", QString(geometry.toBase64()));
        settings->sync();
    }

    QByteArray getMainWindowGeometry() const {
        QString base64Geometry = settings->value("MainWindow/geometry", "").toString();
        return QByteArray::fromBase64(base64Geometry.toLatin1());
    }

    void setMainWindowState(const QByteArray& state) {
        settings->setValue("MainWindow/state", QString(state.toBase64()));
        settings->sync();
    }

    QByteArray getMainWindowState() const {
        QString base64State = settings->value("MainWindow/state", "").toString();
        return QByteArray::fromBase64(base64State.toLatin1());
    }

    // Методы для LoggerWindow
    void setLoggerWindowGeometry(const QByteArray& geometry) {
        settings->setValue("LoggerWindow/geometry", QString(geometry.toBase64()));
        settings->sync();
    }

    QByteArray getLoggerWindowGeometry() const {
        QString base64Geometry = settings->value("LoggerWindow/geometry", "").toString();
        return QByteArray::fromBase64(base64Geometry.toLatin1());
    }

    void setLoggerWindowState(const QByteArray& state) {
        settings->setValue("LoggerWindow/state", QString(state.toBase64()));
        settings->sync();
    }

    QByteArray getLoggerWindowState() const {
        QString base64State = settings->value("LoggerWindow/state", "").toString();
        return QByteArray::fromBase64(base64State.toLatin1());
    }

    void setLoggerSelectedItems(const QStringList& items) {
        settings->setValue("LoggerWindow/selectedItems", items);
        settings->sync();
    }

    QStringList getLoggerSelectedItems() const {
        return settings->value("LoggerWindow/selectedItems", QStringList()).toStringList();
    }

    void setValue(const QString& key, const QVariant& value)
    {
        settings->setValue(key, value);
        settings->sync();
    }

    QVariant value(const QString& key, const QVariant& defaultValue) const
    {
        return settings->value(key, defaultValue);
    }

private:
    Settings() : settings(nullptr) { init(); }
    ~Settings() {
        if (settings) {
            settings->sync();
            delete settings;
        }
    }
    Settings(const Settings&) = delete;
    Settings& operator=(const Settings&) = delete;
    QSettings m_settings;
    QSettings* settings;
};

#endif // SETTINGS_H

#ifndef LOGGERWINDOW_H
#define LOGGERWINDOW_H

#include <QMainWindow>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QTimer>
#include <QMap>
#include <QVector>
#include <QQueue>
#include <QFile>
#include <QTextStream>
#include <QTreeWidgetItem>
#include <QComboBox>
#include <QPushButton>
#include <QTextEdit>
#include "qcustomplot.h"
#include "j2534.h"
#include "loggerdefinitionloader.h"
#include "parameterwidget.h"
#include "parameterselectiondialog.h"
#include "expression.h"
#include <QQueue>
#include "settings.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoggerWindow; }
QT_END_NAMESPACE

// Структура для хранения диагностических запросов
struct DiagRequest {
    uint8_t service;
    uint16_t pid;
};

// Структура для хранения информации о параметрах
struct ParameterInfo {
    QString name;
    QString units;
    QString formula;
    QVector<double> history; // Хранение истории значений
};

class LoggerWindow : public QMainWindow
{
    Q_OBJECT

public:
     LoggerWindow(QWidget *parent = nullptr);
    ~LoggerWindow();

signals:
    void closed(); // Сигнал о закрытии окна
    void connectionEstablished(); // Сигнал об установлении соединения
    void parameterUpdated(const QString& name, const QString& value, const QString& units); // Сигнал об обновлении параметра
    void adapterInitialized(const QString& adapter);

protected:
    void closeEvent(QCloseEvent *event) override; // Переопределение события закрытия окна

private slots:
    void onStartLoggingClicked(); // Обработчик нажатия кнопки "Начать запись"
    void onStopLoggingClicked(); // Обработчик нажатия кнопки "Остановить запись"
    void onReadTimer(); // Обработчик таймера чтения данных
    void onAdapterChanged(const QString& adapter); // Обработчик изменения адаптера
    void onDisplayModeChanged(int index); // Обработчик изменения режима отображения
    void onConnectionEstablished(); // Обработчик установления соединения
    void onParameterItemChanged(QTreeWidgetItem* item, int column); // Обработчик изменения элемента параметра
    void onSaveLogTriggered(); // Обработчик сохранения лога
    void onPreferencesTriggered(); // Обработчик открытия настроек
    void onFileLogButtonClicked(); // Обработчик нажатия кнопки записи в файл
    void updateTimePlot();
    void loadLoggerDefinition(const QString& fileName);
    void onSelectLogDirClicked();
    void onLoadXMLClicked();

private:
    void initializeUI(); // Инициализация UI
    void loadSettings(); // Загрузка настроек
    void saveSettings(); // Сохранение настроек
    void loadAdapters(); // Загрузка доступных адаптеров
    bool initializeConnection(); // Инициализация соединения
    bool initializeAdapter(); // Инициализация адаптера
    bool validateAdapter(); // Валидация выбранного адаптера
    bool initializeJ2534(); // Инициализация J2534
    bool connectToCANChannel(); // Подключение к CAN каналу
    bool setupMessageFilter(); // Настройка фильтра сообщений
    void setupPlot(); // Настройка графика
    void disconnectCurrent(); // Отключение текущего соединения
    bool sendDiagnosticRequest(uint8_t service, uint16_t pid); // Отправка диагностического запроса
    QByteArray readDiagnosticResponse(int timeout = 1000); // Чтение диагностического ответа
    void processResponse(uint8_t requestedService, const QByteArray& response); // Обработка ответа
    bool openDiagnosticSession(); // Открытие диагностической сессии
    void setupRequestQueue(); // Настройка очереди запросов
    void createParameterWidgets(); // Создание виджетов параметров
    void sendNextRequest(); // Отправка следующего запроса
    QString getErrorText(long errorCode); // Получение текста ошибки
    void appendToLog(const QString& message); // Добавление сообщения в лог
    void updateParameter(const QString& name, const QString& value, const QString& units); // Обновление параметра
    void updatePlot(); // Обновление графика
    double calculateValueFromFormula(const ParameterInfo& parameter, const QByteArray& response); // Расчет значения по формуле
    bool m_sessionActive = false; // Флаг для отслеживания состояния сессии
    QQueue<DiagRequest> m_requestQueue;
    QMap<QString, QString> m_adapterToDllMap;
    QMap<QString, QList<ParameterDefinition>> m_parameters;
    QVector<QString> m_selectedParameters; // Список выбранных параметров
    QMap<QString, ParameterWidget*> m_parameterWidgets; // Карта виджетов параметров
    QVector<QCPGraph*> m_graphData;
    void clearCurrentData();
    QColor getNextColor();
    QMap<QString, QWidget*> m_parameterWindows;
    QStackedLayout* m_rightLayout;
    void checkUIState();
    void startLogging();
    void stopLogging();
    bool setupScanmaticFilter();
    LoggerDefinitionLoader m_definitionLoader;
    QMap<QString, ParameterInfo> m_parameterValues;
    void processParameterData(const QString& paramId, const QByteArray& data);
    void updateParameterValue(const QString& name, const QString& value, const QString& units);
    void updateParametersTree();
    QString formatValue(double value, const QString& format);
    bool testAdapterConnection();
    QString m_lastLogDirectory;
    QFile m_logFile;
    QString m_lastXmlFilePath;




    struct CategoryParameters {
        QString name;
        QVector<ParameterDefinition> parameters;
    };
    QMap<QString, CategoryParameters> m_categorizedParameters;
    static constexpr int MAX_HISTORY_SIZE = 1000;

    Ui::LoggerWindow *ui; // Указатель на UI
    J2534* m_j2534; // Указатель на объект J2534
    unsigned long m_deviceId; // Идентификатор устройства
    unsigned long m_channelId; // Идентификатор канала
    unsigned long m_msgId;
    unsigned long m_filterId;    // Идентификатор сообщения
    bool m_isLogging; // Флаг, указывающий, идет ли запись
    bool m_sessionOpened; // Флаг, указывающий, открыта ли с ессия
    QTimer* m_readTimer;     // Таймер для чтения данных
    void setupLoggerDefinitionLoader();

    // Компоненты для графика
    QCustomPlot* m_plot;
    QVector<double> m_timeData;

    // Компоненты UI
    QComboBox* m_adapterComboBox;
    QPushButton* m_startButton;
    QPushButton* m_stopButton;
    QPushButton* m_fileLogButton;
    QTreeWidget* m_parametersTree;
    QTextEdit* m_logTextEdit;
    QScrollArea* m_parametersScrollArea;
    QWidget* m_parametersContainer;
    QVBoxLayout* m_parametersLayout;

    // Компоненты для записи в файл
    bool m_isFileLogging;
    QFile m_csvFile;
    QTextStream m_csvStream;

    // Константы для диагностики
    struct DiagConstants {
        static const uint8_t SESSION_CONTROL = 0x10;
        static const uint8_t READ_DATA_BY_ID = 0x22;
        static const uint16_t EXTENDED_SESSION = 0x03;
        static const int DEFAULT_TIMEOUT = 1000;
        static const int SESSION_TIMEOUT = 2000;
    };
};

#endif // LOGGERWINDOW_H

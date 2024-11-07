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
#include "parameterselectiondialog.h"
#include <QDomNodeList>
#include "settings.h"
#include "expression.h"
#include "parametergraphicswidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoggerWindow; }
QT_END_NAMESPACE

// Структура для хранения диагностических запросов
struct DiagRequest {
    uint8_t service; // Номер сервиса диагностического запроса.
    uint16_t pid; // Параметр идентификатора (PID) диагностического запроса.
};

// Структура для хранения информации о параметрах
struct ParameterInfo {
    QString name; // Наименование параметра.
    QString units; // Единицы измерения параметра.
    QString formula; // Формула для расчета значения параметра.
    QVector<double> history; // История значений параметра.
};

class LoggerWindow : public QMainWindow // Класс LoggerWindow, наследуемый от QMainWindow.
{
    Q_OBJECT // Макрос, необходимый для использования сигналов и слотов.

public:
    LoggerWindow(QWidget *parent = nullptr); // Конструктор класса.
    ~LoggerWindow(); // Деструктор класса.
    Parameter getParameterByName(const QString& name) const;

signals:
    void closed(); // Сигнал, генерируемый при закрытии окна.
    void connectionEstablished(); // Сигнал, генерируемый при установлении соединения.
    void parameterUpdated(const QString& name, double value, const QString& units);   // Сигнал, генерируемый при обновлении значения параметра.
    void adapterInitialized(const QString& adapter); // Сигнал, генерируемый при инициализации адаптера.
    //void windowSizeChanged(ParameterWidget::WindowSize size);

protected:
    void closeEvent(QCloseEvent *event) override; // Переопределение метода closeEvent для обработки события закрытия окна.

private slots:
    void onStartLoggingClicked(); // Слот, вызываемый при нажатии кнопки "Начать запись".
    void onStopLoggingClicked(); // Слот, вызываемый при нажатии кнопки "Остановить запись".
    void onReadTimer(); // Слот, вызываемый по таймеру для чтения данных.
    void onAdapterChanged(const QString& adapter); // С лот, вызываемый при изменении адаптера.
    void onDisplayModeChanged(int index); // Слот, вызываемый при изменении режима отображения.
    void onConnectionEstablished(); // Слот, вызываемый при установлении соединения.
    void onParameterItemChanged(QTreeWidgetItem* item, int column); // Слот, вызываемый при изменении элемента параметра.
    void onSaveLogTriggered(); // Слот, вызываемый при сохранении лога.
    void onPreferencesTriggered(); // Слот, вызываемый при открытии настроек.
    void onFileLogButtonClicked(); // Слот, вызываемый при нажатии кнопки записи в файл.
    void updateTimePlot(); // Слот для обновления графика времени.
    void loadLoggerDefinition(const QString& fileName); // Слот для загрузки определения логгера из файла.
    void onSelectLogDirClicked(); // Слот, вызываемый при выборе директории для логирования.
    void onLoadXMLClicked(); // Слот, вызываемый при загрузке XML-файла.
    void onParameterUnitsChanged(int index); // Слот, вызываемый при изменении единиц измерения параметра.
    void onParameterChecked(int state); // Слот для обработки изменения состояния чекбокса.
    void onParameterUpdated(const ParameterDefinition& param, const QString& value, const QString& units);

private:
    void initializeUI(); // Метод для инициализации пользовательского интерфейса.
    void loadSettings(); // Метод для загрузки настроек приложения.
    void saveSettings(); // Метод для сохранения настроек приложения.
    void loadAdapters(); // Метод для загрузки доступных адаптеров.
    bool initializeConnection(); // Метод для инициализации соединения.
    bool initializeAdapter(); // Метод для инициализации адаптера.
    bool validateAdapter(); // Метод для валидации выбранного адаптера.
    bool initializeJ2534(); // Метод для инициализации J2534.
    bool connectToCANChannel(); // Метод для подключения к CAN каналу.
    bool setupMessageFilter(); // Метод для настройки фильтра сообщений.
    void setupPlot(); // Метод для настройки графика.
    void disconnectCurrent(); // Метод для отключения текущего соединения.
    bool sendDiagnosticRequest(uint8_t service, uint16_t pid); // Метод для отправки диагностического запроса.
    QByteArray readDiagnosticResponse(int timeout = 1000); // Метод для чтения диагностического ответа.
    void processResponse(uint8_t requestedService, const QByteArray& response); // Метод для обработки ответа.
    bool openDiagnosticSession(); // Метод для открытия диагностической сессии.
    void setupRequestQueue(); // Метод для настройки очереди запросов.

    void sendNextRequest(); // Метод для отправки следующего запроса.
    QString getErrorText(long errorCode); // Метод для получения текста ошибки.
    void appendToLog(const QString& message); // Метод для добавления сообщения в лог.
    void updateParameter(const ParameterDefinition& param, const QString& value, const QString& units); // Метод для обновления параметра.
    void updatePlot(); // Метод для обновления графика.
    double calculateValueFromFormula(const ParameterInfo& parameter, const QByteArray& response); // Метод для расчета значения по формуле.
    bool m_sessionActive = false; // Флаг для отслеживания состояния сессии.
    QQueue<DiagRequest> m_requestQueue; // Очередь диагностических запросов.
    QMap<QString, QString> m_adapterToDllMap; // Карта соответствия адаптеров и их DLL.
    QMap<QString, QList<ParameterDefinition>> m_parameters; // Карта параметров.
    QVector<Parameter> m_selectedParameters; // Список выбранных параметров.
    QVector<Parameter> allParameters;
    //QMap<QString, ParameterWidget*> m_parameterWidgets; // Карта виджетов параметров.
    //QMap<QString, ParameterWidget*> m_parameterWindows; // Карта для хранения окон параметров
    QGraphicsScene* m_parametersScene;
    QMap<QString, ParameterGraphicsWidget*> m_parameterGraphicsWidgets; // Карта виджетов параметров
    QVector<QCPGraph*> m_graphData; // Данные для графиков.
    void clearCurrentData(); // Метод для очистки текущих данных.
    QColor getNextColor(); // Метод для получения следующего цвета для графика.
    QStackedLayout* m_rightLayout; // Компоновка для правой части интерфейса.
    void checkUIState(); // Метод для проверки состояния пользовательского интерфейса.
    void startLogging(); // Метод для начала записи данных.
    void stopLogging(); // Метод для остановки записи данных.
    bool setupScanmaticFilter(); // Метод для настройки фильтра Scanmatic.
    LoggerDefinitionLoader m_definitionLoader; // Объект для загрузки определения логгера.
    QMap<QString, ParameterInfo> m_parameterValues; // Карта значений параметров.
    void processParameterData(const QString& paramId, const QByteArray& data); // Метод для обработки данных параметра.
    void updateParameterValue(const ConfigManager::Parameter* param, const QString& value, const QString& units); // Метод для обновления значения параметра.
    void updateParametersTree(); // Метод для обновления дерева параметров.
    QString formatValue(double value, const QString& format); // Метод для форматирования значения.
    bool testAdapterConnection(); // Метод для тестирования соединения с адаптером.
    QString m_lastLogDirectory; // Последняя директория для логирования.
    QFile m_logFile; // Файл для логирования.
    QString m_lastXmlFilePath; // Последний путь к XML-файлу.
    void filterParameters(const QString& filterText); // Метод для фильтрации параметров по тексту.
    void loadParameters(const QDomNodeList& parameterNodes, const QString& category); // Метод для загрузки параметров из списка узлов DOM.
    void hideAllCheckboxes(); // Метод для скрытия всех чекбоксов.
    void hideCheckboxesForItem(QTreeWidgetItem *item); // Метод для скрытия чекбоксов для конкретного элемента.
    void showCheckboxForItem(QTreeWidgetItem *item); // Метод для отображения чекбокса для конкретного элемента.
    int columns; // Количество столбцов в сетке
    void rearrangeParameterWindows(); // Функция для перераспределения окон
    QPushButton* m_windowSizeButton;
    void onWindowSizeButtonClicked();
    void createParameterGraphicsWidgets(); // Метод для создания графических виджетов параметров
    void updateParameterGraphicsWidgets(); // Метод для обновления графических виджетов параметров
    void rearrangeParameterGraphicsWidgets();


    struct CategoryParameters { // Структура для хранения параметров категории.
        QString name; // Наименование категории.
        QVector<ParameterDefinition> parameters; // Параметры в категории.
    };
    QMap<QString, CategoryParameters> m_categorizedParameters; // Карта категорий параметров.
    static constexpr int MAX_HISTORY_SIZE = 1000; // Максимальный размер истории значений.

    Ui::LoggerWindow *ui; // Указатель на UI.
    J2534* m_j2534; // Указатель на объект J2534.
    unsigned long m_deviceId; // Идентификатор устройства.
    unsigned long m_channelId; // Идентификатор канала.
    unsigned long m_msgId; // Идентификатор сообщения.
    unsigned long m_filterId; // Идентификатор фильтра сообщения.
    bool m_isLogging; // Флаг, указывающий, идет ли запись.
    bool m_sessionOpened; // Флаг, указывающий, открыта ли сессия.
    QTimer* m_readTimer; // Таймер для чтения данных.
    void setupLoggerDefinitionLoader(); // Метод для настройки загрузчика определения логгера.

    // Компоненты для графика
    QCustomPlot* m_plot; // Указатель на объект графика.
    QVector<double> m_timeData; // Данные времени для графика.

    // Компоненты UI
    QComboBox* m_adapterComboBox; // Выпадающий список для выбора адаптера.
    QPushButton* m_startButton; // Кнопка для начала записи.
    QPushButton* m_stopButton; // Кнопка для остановки записи.
    QPushButton* m_fileLogButton; // Кнопка для записи в файл.
    QTreeWidget* m_parametersTree; // Дерево для отображения параметров.
    QTextEdit* m_logTextEdit; // Поле для отображения логов.
    QScrollArea* m_parametersScrollArea; // Область прокрутки для параметров.
    QWidget* m_parametersContainer; // Контейнер для параметров.
    QVBoxLayout* m_parametersLayout; // Вертикальный макет для параметров.

    // Компоненты для записи в файл
    bool m_isFileLogging; // Флаг, указывающий, идет ли запись в файл.
    QFile m_csvFile; // Файл для записи в формате CSV.
    QTextStream m_csvStream; // Поток для записи в CSV-файл.

    // Константы для диагностики
    struct DiagConstants { // Структура для хранения констант диагностики.
        static const uint8_t SESSION_CONTROL = 0x10; // Код управления сессией.
        static const uint8_t READ_DATA_BY_ID = 0x22; // Код чтения данных по идентификатору.
        static const uint16_t EXTENDED_SESSION = 0x03; // Код расширенной сессии.
        static const int DEFAULT_TIMEOUT = 1000; // Значение таймаута по умолчанию.
        static const int SESSION_TIMEOUT = 2000; // Таймаут сессии.
    };
};

#endif // LOGGERWINDOW_H

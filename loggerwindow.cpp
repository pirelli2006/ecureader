#include "loggerwindow.h"
#include "configmanager.h"
#include "ui_loggerwindow.h"
#include <QMessageBox>
#include <QDateTime>
#include <QCloseEvent>
#include <QSettings>
#include <QDir>
#include <QTimer>
#include <QThread>
#include <QElapsedTimer>
#include <QFileInfo>
#include "ParameterWindow.h"
#include <QFileDialog>
#include <QTextStream>
#include <QSettings>
#include "preferencesdialog.h"
#include <QJSEngine>
#include <QVector>
#include "qcustomplot.h"
#include <QQueue>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QPushButton>
#include <QStringList>
#include <QDebug>
#include "expression.h"
#include <QDomDocument>
#include <QDomElement>
#include "parameter.h"


LoggerWindow::LoggerWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::LoggerWindow),
    m_j2534(nullptr),
    m_deviceId(0),
    m_channelId(0),
    m_msgId(0),
    m_filterId(0),
    m_isLogging(false),
    m_sessionOpened(false),
    m_readTimer(new QTimer(this)),
    m_isFileLogging(false)
{
    qDebug() << "Starting LoggerWindow constructor";

    ui->setupUi(this);
    qDebug() << "UI setup completed";

    // Используем существующий QCustomPlot из формы
    m_plot = ui->plotWidget;
    if (!m_plot) {
        qCritical() << "Failed to get plot widget!";
        return;
    }
    qDebug() << "Plot assigned";

    setupPlot();
    qDebug() << "Plot setup completed";

    setupLoggerDefinitionLoader();

    // Настраиваем комбобокс режима отображения
    ui->displayModeComboBox->clear();
    ui->displayModeComboBox->addItem("Widgets");
    ui->displayModeComboBox->addItem("Plot");
    connect(ui->displayModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &LoggerWindow::onDisplayModeChanged);
    connect(ui->startButton, &QPushButton::clicked, this, &LoggerWindow::onStartLoggingClicked);
    connect(ui->stopButton, &QPushButton::clicked, this, &LoggerWindow::onStopLoggingClicked);
    connect(this, &LoggerWindow::connectionEstablished, this, &LoggerWindow::onConnectionEstablished);

    // Синхронизация начального состояния отображения
    onDisplayModeChanged(ui->displayModeComboBox->currentIndex());

    qDebug() << "Display mode combo box setup completed";

    initializeUI();
    qDebug() << "UI initialization completed";

    connect(this, &LoggerWindow::connectionEstablished, this, &LoggerWindow::onConnectionEstablished);

    checkUIState();
    qDebug() << "Constructor completed";

    // Настройка начального состояния кнопок
    ui->startButton->setEnabled(true);
    ui->stopButton->setEnabled(false);
    ui->fileLogButton->setEnabled(false);
}

LoggerWindow::~LoggerWindow()
{
    if (m_isLogging) {
        qDebug() << "Stopping logging from destructor";
        onStopLoggingClicked();
    }

    if (m_csvFile.isOpen()) {
        m_csvFile.close();
    }

    if (m_readTimer) {
        m_readTimer->stop();
        delete m_readTimer;
        m_readTimer = nullptr;
    }

    // Очистка виджетов параметров
    qDeleteAll(m_parameterWidgets);
    m_parameterWidgets.clear();

    disconnectCurrent();

    if (ui) {
        delete ui;
        ui = nullptr;
    }
}

void LoggerWindow::closeEvent(QCloseEvent *event)
{
    if (m_isLogging) {
        onStopLoggingClicked();
    }
    if (m_isFileLogging) {
        m_csvFile.close();
    }
    emit closed();
    event->accept();
}

void LoggerWindow::initializeUI()
{
    // Инициализация указателей на UI элементы
    m_adapterComboBox = ui->adapterComboBox;
    m_startButton = ui->startButton;
    m_stopButton = ui->stopButton;
    m_fileLogButton = ui->fileLogButton;
    m_parametersTree = ui->parametersTree;
    m_logTextEdit = ui->logTextEdit;
    m_plot = ui->plotWidget;

    // Загрузка адаптеров
    loadAdapters();

    // Настройка начального состояния
    m_stopButton->setEnabled(false);
    m_fileLogButton->setEnabled(false);

    // Настройка комбобокса режима отображения
    ui->displayModeComboBox->clear();
    ui->displayModeComboBox->addItem("Widgets");
    ui->displayModeComboBox->addItem("Plot");

    // Инициализация области параметров
    m_parametersContainer = new QWidget(this);
    m_parametersLayout = new QVBoxLayout(m_parametersContainer);

    // Настройка дерева параметров
    m_parametersTree->setColumnCount(4);
    m_parametersTree->setHeaderLabels(QStringList() << "" << "Parameter" << "Value" << "Units");

    setupPlot();
}

void LoggerWindow::updateTimePlot()
{
    // Обновление диапазона оси X
    double currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
    m_plot->xAxis->setRange(currentTime - 30, currentTime);
    m_plot->replot();
}
void LoggerWindow::onPreferencesTriggered()
{
    // Создание и отображение диалога настроек
    // Например:
    PreferencesDialog preferencesDialog(this);
    if (preferencesDialog.exec() == QDialog::Accepted)
    {
        // Применить новые настройки
        loadSettings();
    }
}

void LoggerWindow::onFileLogButtonClicked()
{
    // Здесь должна быть реализация функции
    // Например:
    QString fileName = QFileDialog::getSaveFileName(this, tr("Save Log File"), "", tr("Log Files (*.log);;All Files (*)"));
    if (fileName.isEmpty())
        return;
    else {
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::information(this, tr("Unable to open file"), file.errorString());
            return;
        }
        QTextStream out(&file);
        out << ui->logTextEdit->toPlainText();
    }
}

void LoggerWindow::loadSettings()
{
    QSettings settings;

    // Загрузка настроек
    // Например:
    settings.beginGroup("Logger");
    // Чтение настроек
    bool autoScroll = settings.value("AutoScroll", true).toBool();
    int maxLines = settings.value("MaxLines", 1000).toInt();
    // и т.д.
    settings.endGroup();

    // Применение загруженных настроек
    // Например:
    // setAutoScroll(autoScroll);
    // setMaxLines(maxLines);
}

void LoggerWindow::onSaveLogTriggered()
{
    QString fileName = QFileDialog::getSaveFileName(this,
                                                    tr("Save Log File"), "",
                                                    tr("Log Files (*.log);;All Files (*)"));

    if (fileName.isEmpty())
        return;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::warning(this, tr("Error"), tr("Cannot open file for writing."));
        return;
    }

    QTextStream out(&file);
    // Здесь запишите содержимое лога в файл
    // Например, если у вас есть QTextEdit* m_logTextEdit:
    // out << m_logTextEdit->toPlainText();

    file.close();
    QMessageBox::information(this, tr("Success"), tr("Log file has been saved successfully."));
}

void LoggerWindow::loadAdapters()
{
    static bool isLoading = false;
    if (isLoading) return;
    isLoading = true;

    // Очищаем список и карту перед загрузкой
    ui->adapterComboBox->clear();
    m_adapterToDllMap.clear();

    QSettings settings32("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\PassThruSupport.04.04",
                         QSettings::NativeFormat);
    QSettings settings64("HKEY_LOCAL_MACHINE\\SOFTWARE\\PassThruSupport.04.04",
                         QSettings::NativeFormat);

    auto loadFromSettings = [this](QSettings& settings) {
        for (const QString& vendor : settings.childGroups()) {
            settings.beginGroup(vendor);
            QString name = settings.value("Name").toString();
            QString dll = settings.value("FunctionLibrary").toString();
            if (!name.isEmpty() && !dll.isEmpty()) {
                // Проверяем, не добавлен ли уже этот адаптер
                if (ui->adapterComboBox->findText(name) == -1) {
                    ui->adapterComboBox->addItem(name);
                    m_adapterToDllMap[name] = dll;
                }
            }
            settings.endGroup();
        }
    };

    loadFromSettings(settings32);
    loadFromSettings(settings64);

    isLoading = false;
}

void LoggerWindow::onStartLoggingClicked()
{
    qDebug() << "Start button clicked";
    appendToLog("Starting logging process...");

    // Блокируем кнопку старта
    ui->startButton->setEnabled(false);

    try {
        // Инициализация соединения
        if (!initializeConnection()) {
            QMessageBox::critical(this, "Error", "Failed to initialize connection");
            appendToLog("Failed to initialize connection");
            // Возвращаем кнопки в исходное состояние
            ui->startButton->setEnabled(true);
            ui->stopButton->setEnabled(false);
            ui->fileLogButton->setEnabled(false);
            return;
        }

        // Открытие диагностической сессии
        if (!openDiagnosticSession()) {
            QMessageBox::critical(this, "Error", "Failed to open diagnostic session");
            appendToLog("Failed to open diagnostic session");
            disconnectCurrent();
            // Возвращаем кнопки в исходное состояние
            ui->startButton->setEnabled(true);
            ui->stopButton->setEnabled(false);
            ui->fileLogButton->setEnabled(false);
            return;
        }

        // Активируем кнопки стоп и запись лога
        ui->stopButton->setEnabled(true);
        ui->fileLogButton->setEnabled(true);

        m_sessionActive = true;
        appendToLog("Logging process started successfully");

        emit connectionEstablished();

    } catch (const std::exception& e) {
        QMessageBox::critical(this, "Error", QString("Exception occurred: %1").arg(e.what()));
        appendToLog(QString("Error: %1").arg(e.what()));
        disconnectCurrent();
        // Возвращаем кнопки в исходное состояние
        ui->startButton->setEnabled(true);
        ui->stopButton->setEnabled(false);
        ui->fileLogButton->setEnabled(false);
    }
}

void LoggerWindow::onStopLoggingClicked()
{
    if (!m_sessionActive) {
        appendToLog("No active session to stop.");
        return;
    }

    appendToLog("Stopping logging process...");

    // Закрытие сессии и отключение от устройства
    disconnectCurrent();

    m_sessionActive = false; // Сбрасываем флаг активности
    appendToLog("Logging process stopped.");

    // Активируем кнопку "Старт"
    ui->startButton->setEnabled(true);
    // Деактивируем кнопку "Стоп"
    ui->stopButton->setEnabled(false);
}

void LoggerWindow::onReadTimer()
{
    if (m_requestQueue.isEmpty()) {
        sendNextRequest();
    }
    if (m_isLogging) {
        sendDiagnosticRequest(m_requestQueue.head().service, m_requestQueue.head().pid);
        m_requestQueue.dequeue();
    }
}

void LoggerWindow::onAdapterChanged(const QString& adapter)
{
    if (adapter.isEmpty()) {
        return;
    }
    m_adapterComboBox->setCurrentText(adapter);
    if (!initializeAdapter()) {
        QMessageBox::critical(this, "Error", "Failed to initialize adapter");
    }
}

void LoggerWindow::appendToLog(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->logTextEdit->append(QString("[%1] %2").arg(timestamp).arg(message));
}

void LoggerWindow::disconnectCurrent()
{
    if (m_channelId) {
        if (m_filterId) {
            m_j2534->PassThruStopMsgFilter(m_channelId, m_filterId);
            m_filterId = 0;
        }
        m_j2534->PassThruDisconnect(m_channelId);
        m_channelId = 0;
    }

    if (m_deviceId) {
        m_j2534->PassThruClose(m_deviceId);
        m_deviceId = 0;
    }

    m_sessionActive = false;
    appendToLog("Disconnected from device");
}

bool LoggerWindow::sendDiagnosticRequest(uint8_t service, uint16_t pid)
{
    if (!m_j2534 || !m_channelId) {
        appendToLog("Error: Device not properly initialized");
        return false;
    }

    PASSTHRU_MSG msg = {0};
    msg.ProtocolID = ISO15765;
    msg.TxFlags = ISO15765_FRAME_PAD;

    // CAN ID setup (0x7E0)
    msg.Data[0] = 0x00;
    msg.Data[1] = 0x00;
    msg.Data[2] = 0x07;
    msg.Data[3] = 0xE0;

    // Service and PID
    msg.Data[4] = service;
    if (service == DiagConstants::SESSION_CONTROL) {
        msg.Data[5] = static_cast<uint8_t>(pid & 0xFF);
        msg.DataSize = 6;
    } else {
        msg.Data[5] = static_cast<uint8_t>(pid & 0xFF);
        msg.DataSize = 6;
    }

    unsigned long numMsgs = 1;
    long status = m_j2534->PassThruWriteMsgs(m_channelId, &msg, &numMsgs, DiagConstants::DEFAULT_TIMEOUT);

    if (status != STATUS_NOERROR) {
        appendToLog("Error sending message: " + getErrorText(status));
        return false;
    }

    return true;
}

QByteArray LoggerWindow::readDiagnosticResponse(int timeout)
{
    QByteArray response;
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeout) {
        PASSTHRU_MSG msg = {0};
        unsigned long numMsgs = 1;

        long status = m_j2534->PassThruReadMsgs(m_channelId, &msg, &numMsgs, 100);

        if (status != STATUS_NOERROR) {
            if (status != ERR_TIMEOUT && status != ERR_BUFFER_EMPTY) {
                appendToLog("Read error: " + getErrorText(status));
            }
            continue;
        }

        if (numMsgs == 0) continue;

        // Проверка валидного ответа (0x7E8)
        if (msg.DataSize >= 4 &&
            msg.Data[0] == 0x00 && msg.Data[1] == 0x00 &&
            msg.Data[2] == 0x07 && msg.Data[3] == 0xE8) {

            response = QByteArray((char*)&msg.Data[4], msg.DataSize - 4);
            break;
        }

        QThread::msleep(10);
    }

    return response;
}

void LoggerWindow::processResponse(uint8_t requestedService, const QByteArray& response)
{
    if (response.isEmpty()) {
        appendToLog("Empty response received");
        return;
    }

    uint8_t responseService = static_cast<uint8_t>(response[0]);

    if (responseService == requestedService + 0x40) {
        // Positive response
        QString dataHex;
        for (int i = 1; i < response.size(); ++i) {
            dataHex += QString("%1 ").arg(static_cast<uint8_t>(response[i]), 2, 16, QChar('0'));
        }
        appendToLog(QString("Response: %1").arg(dataHex.trimmed()));

        // Обработка параметров на основе XML
        if (requestedService == 0x21) {
            uint8_t pid = response[1];
            QString pidKey = QString("%1").arg(pid, 2, 16, QChar('0')).toUpper();

            if (m_parameters.contains(pidKey) && !m_parameters[pidKey].isEmpty()) {
                const ParameterDefinition& param = m_parameters[pidKey].first();

                ParameterInfo tempInfo;
                tempInfo.name = param.name;
                tempInfo.units = param.units;
                tempInfo.formula = param.expression;

                double value = calculateValueFromFormula(tempInfo, response);
                value = qBound(param.min, static_cast<float>(value), param.max);

                QString valueStr;
                if (!param.format.isEmpty()) {
                    if (param.format.contains('f')) {
                        // Для чисел с плавающей точкой
                        int precision = param.format.mid(param.format.indexOf('.') + 1, 1).toInt();
                        valueStr = QString("%1").arg(value, 0, 'f', precision);
                    } else if (param.format.contains('d')) {
                        // Для целых чисел
                        valueStr = QString("%1").arg(static_cast<int>(value));
                    } else {
                        // Для других форматов
                        valueStr = QString("%1").arg(value);
                    }
                } else {
                    valueStr = QString::number(value);
                }

                updateParameter(param.name, valueStr, param.units);
            }
            updatePlot();
        }
    } else if (responseService == 0x7F) {
        appendToLog(QString("Negative response: NRC %1")
                        .arg(static_cast<uint8_t>(response[2]), 2, 16, QChar('0')));
    }
}

double LoggerWindow::calculateValueFromFormula(const ParameterInfo& parameter, const QByteArray& response)
{
    QString formula = parameter.formula;

    // Заменяем переменные в формуле на значения из response
    for (int i = 0; i < response.size() - 1; ++i) {
        QString varName = QString("A%1").arg(i);
        int value = static_cast<uint8_t>(response[i + 1]);
        formula.replace(varName, QString::number(value));
    }

    // Вычисляем результат
    QJSEngine engine;
    QJSValue result = engine.evaluate(formula);

    if (result.isError()) {
        qWarning() << "Error evaluating formula:" << formula;
        return 0.0;
    }

    return result.toNumber();
}

bool LoggerWindow::openDiagnosticSession()
{
    if (!sendDiagnosticRequest(DiagConstants::SESSION_CONTROL, DiagConstants::EXTENDED_SESSION)) {
        appendToLog("Failed to send session control request");
        return false;
    }

    QByteArray response = readDiagnosticResponse(DiagConstants::SESSION_TIMEOUT);
    if (response.isEmpty()) {
        appendToLog("No response for session control request");
        return false;
    }

    if (!response.isEmpty() && static_cast<unsigned char>(response[0]) != DiagConstants::SESSION_CONTROL + 0x40) {
        appendToLog("Invalid session control response");
        return false;
    }

    m_sessionOpened = true;
    appendToLog("Diagnostic session opened");
    return true;
}

void LoggerWindow::setupRequestQueue()
{
    m_requestQueue.clear();

    // Добавляем в очередь только выбранные параметры
    for (const auto& paramName : m_selectedParameters) {
        const auto* param = ConfigManager::instance().getParameter(paramName);
        if (param) {
            auto* widget = new ParameterWidget(
                param->name,
                param->conversions.first().units,
                param->conversions.first().gauge_min,
                param->conversions.first().gauge_max,
                m_parametersContainer
                );
            m_parameterWidgets[param->name] = widget;
            m_parametersLayout->insertWidget(m_parametersLayout->count() - 1, widget);
        }
    }
}

void LoggerWindow::onDisplayModeChanged(int index)
{
    qDebug() << "Display mode changed to index:" << index;

    if (index == 0) {  // Widgets mode
        ui->tabWidget->setCurrentWidget(ui->dashboardTab);
        ui->plotWidget->hide();
        qDebug() << "Switched to Widgets mode";
    } else if (index == 1) {  // Plot mode
        ui->tabWidget->setCurrentWidget(ui->dashboardTab);
        ui->plotWidget->show();
        qDebug() << "Switched to Plot mode";
    }

    checkUIState();
}

void LoggerWindow::checkUIState()
{
    qDebug() << "Checking UI state:";
    qDebug() << "plotWidget visibility:" << ui->plotWidget->isVisible();
    qDebug() << "parametersTree:" << (ui->parametersTree != nullptr);
    qDebug() << "adapterComboBox:" << (ui->adapterComboBox != nullptr);
    qDebug() << "Current display mode index:" << ui->displayModeComboBox->currentIndex();
}


void LoggerWindow::onConnectionEstablished()
{
    qDebug() << "Connection established";
    appendToLog("Connection established, preparing parameter selection...");

    // Получаем параметры из ConfigManager
    QVector<ConfigManager::Parameter> parameters = ConfigManager::instance().getAllParameters();

    if (parameters.isEmpty()) {
        QMessageBox::warning(this, "Warning", "No parameters available");
        appendToLog("No parameters available for logging");
        disconnectCurrent();
        ui->startButton->setEnabled(true);
        return;
    }

    // Создаем диалог выбора параметров
    ParameterSelectionDialog dialog(parameters, this);

    if (dialog.exec() == QDialog::Accepted) {
        // Получаем выбранные параметры
        m_selectedParameters = QVector<QString>::fromList(dialog.getSelectedParameters());

        if (m_selectedParameters.isEmpty()) {
            QMessageBox::warning(this, "Warning", "No parameters selected");
            appendToLog("No parameters selected for logging");
            disconnectCurrent();
            ui->startButton->setEnabled(true);
            return;
        }

        // Создаем виджеты для выбранных параметров
        createParameterWidgets();

        // Настраиваем очередь запросов
        setupRequestQueue();

        // Запускаем таймер для чтения данных
        if (m_readTimer) {
            m_readTimer->start(100);
        }

        m_isLogging = true;
        appendToLog("Logging started with " + QString::number(m_selectedParameters.size()) + " parameters");

    } else {
        appendToLog("Parameter selection cancelled");
        disconnectCurrent();
        ui->startButton->setEnabled(true);
    }
}

bool LoggerWindow::setupScanmaticFilter()
{
    qDebug() << "Setting up Scanmatic filter...";
    appendToLog("Setting up Scanmatic filter...");

    PASSTHRU_MSG maskMsg = {0}, patternMsg = {0}, flowControlMsg = {0};

    // Настройка маски
    maskMsg.ProtocolID = ISO15765;
    maskMsg.Data[0] = maskMsg.Data[1] = maskMsg.Data[2] = maskMsg.Data[3] = 0xFF;
    maskMsg.DataSize = 4;

    // Настройка шаблона для приема
    patternMsg.ProtocolID = ISO15765;
    patternMsg.Data[0] = 0x00;
    patternMsg.Data[1] = 0x00;
    patternMsg.Data[2] = 0x07;
    patternMsg.Data[3] = 0xE8;  // ID ответа от ECU
    patternMsg.DataSize = 4;

    // Настройка Flow Control
    flowControlMsg.ProtocolID = ISO15765;
    flowControlMsg.Data[0] = 0x00;
    flowControlMsg.Data[1] = 0x00;
    flowControlMsg.Data[2] = 0x07;
    flowControlMsg.Data[3] = 0xE0;  // ID запроса к ECU
    flowControlMsg.DataSize = 4;

    long status = m_j2534->PassThruStartMsgFilter(
        m_channelId,
        FLOW_CONTROL_FILTER,
        &maskMsg,
        &patternMsg,
        &flowControlMsg,
        &m_filterId
        );

    if (status != STATUS_NOERROR) {
        QString errorMsg = QString("Failed to set Scanmatic filter: %1").arg(getErrorText(status));
        qDebug() << errorMsg;
        appendToLog(errorMsg);
        return false;
    }

    qDebug() << "Scanmatic filter set successfully";
    appendToLog("Scanmatic filter set successfully");
    return true;
}

void LoggerWindow::clearCurrentData()
{
    // Очищаем дерево параметров
    ui->parametersTree->clear();

    // Очищаем графики
    m_plot->clearGraphs();
    m_graphData.clear();

    // Очищаем выбранные параметры
    m_selectedParameters.clear();
}

QColor LoggerWindow::getNextColor()
{
    // Массив предопределенных цветов
    static const QColor colors[] = {
        Qt::blue, Qt::red, Qt::green, Qt::cyan, Qt::magenta,
        Qt::yellow, Qt::darkBlue, Qt::darkRed, Qt::darkGreen,
        Qt::darkCyan, Qt::darkMagenta, Qt::darkYellow
    };
    static int colorIndex = 0;

    QColor color = colors[colorIndex % (sizeof(colors) / sizeof(colors[0]))];
    colorIndex++;
    return color;
}

void LoggerWindow::createParameterWidgets()
{
    // Очищаем существующие виджеты
    for (auto widget : m_parameterWidgets) {
        delete widget;
    }
    m_parameterWidgets.clear();

    // Создаем новые виджеты для выбранных параметров
    for (const auto& paramName : m_selectedParameters) {
        const auto* param = ConfigManager::instance().getParameter(paramName);
        if (param && !param->conversions.isEmpty()) {
            const auto& conv = param->conversions.first();
            auto* widget = new ParameterWidget(
                param->name,
                conv.units,
                conv.gauge_min,
                conv.gauge_max,
                m_parametersContainer
                );
            m_parameterWidgets[param->name] = widget;
            m_parametersLayout->insertWidget(m_parametersLayout->count() - 1, widget);
        }
    }
}

void LoggerWindow::sendNextRequest()
{
    if (m_requestQueue.isEmpty()) return;

    DiagRequest request = m_requestQueue.dequeue();
    if (sendDiagnosticRequest(request.service, request.pid)) {
        QByteArray response = readDiagnosticResponse();
        processResponse(request.service, response);
    }
}

QString LoggerWindow::getErrorText(long errorCode)
{
    char errorMsg[512] = {0};
    if (m_j2534) {
        m_j2534->PassThruGetLastError(errorMsg);
        return QString("Error %1: %2").arg(errorCode).arg(errorMsg);
    }
    return QString("Error %1").arg(errorCode);
}

bool LoggerWindow::initializeConnection()
{
    if (m_sessionActive) {
        qDebug() << "Connection already initialized";
        appendToLog("Connection already initialized");
        return true;
    }

    qDebug() << "Initializing connection...";
    appendToLog("Initializing connection...");

    // Проверка выбранного адаптера
    if (!validateAdapter()) {
        appendToLog("No valid adapter available");
        return false;
    }

    // Инициализация J2534
    if (!initializeJ2534()) {
        appendToLog("Failed to initialize J2534");
        return false;
    }

    // Подключение к CAN каналу
    if (!connectToCANChannel()) {
        appendToLog("Failed to connect to CAN channel");
        disconnectCurrent();
        return false;
    }

    // Настройка фильтров сообщений для Сканматик
    if (!setupScanmaticFilter()) {
        appendToLog("Failed to setup Scanmatic filter");
        disconnectCurrent();
        return false;
    }

    appendToLog("Connection initialized successfully");
    return true;
}

bool LoggerWindow::initializeAdapter()
{
    if (!m_j2534 || !m_deviceId) {
        appendToLog("Device not properly initialized");
        return false;
    }

    // Здесь можно добавить дополнительную инициализацию адаптера, если необходимо

    return true;
}

bool LoggerWindow::validateAdapter()
{
    QString adapterName = ui->adapterComboBox->currentText();
    if (ui->adapterComboBox->count() == 0 || adapterName.isEmpty()) {
        appendToLog("No adapters available in the system");
        return false;
    }

    QString dllPath = m_adapterToDllMap.value(adapterName);
    if (dllPath.isEmpty() || !QFileInfo(dllPath).exists()) {
        appendToLog("DLL file not found for adapter: " + adapterName);
        return false;
    }

    return true;
}

bool LoggerWindow::initializeJ2534()
{
    QString adapterName = ui->adapterComboBox->currentText();
    QString dllPath = m_adapterToDllMap.value(adapterName);

    appendToLog("Initializing J2534...");
    m_j2534 = new J2534(dllPath);
    if (m_j2534 && !m_j2534->init()) {
        appendToLog("Failed to initialize J2534 DLL: " + dllPath);
        delete m_j2534;
        m_j2534 = nullptr;
        return false;
    }

    appendToLog("Opening device...");
    long status = m_j2534->PassThruOpen(nullptr, &m_deviceId);
    if (status != STATUS_NOERROR) {
        appendToLog("Failed to open device: " + getErrorText(status));
        delete m_j2534;
        m_j2534 = nullptr;
        return false;
    }

    return initializeAdapter();
}

bool LoggerWindow::connectToCANChannel()
{
    qDebug() << "Connecting to CAN channel...";

    if (!m_j2534 || m_deviceId == 0) {
        qDebug() << "J2534 not initialized or invalid device ID";
        return false;
    }

    // Настройка параметров канала
    long status = m_j2534->PassThruConnect(m_deviceId, ISO15765, 0, 500000, &m_channelId);
    if (status != STATUS_NOERROR) {
        QString errorMsg = QString("Failed to connect to CAN channel: %1").arg(getErrorText(status));
        qDebug() << errorMsg;
        appendToLog(errorMsg);
        return false;
    }

    // Установка параметров канала
    SCONFIG_LIST configList = {0};
    SCONFIG configs[4] = {0};

    configs[0].Parameter = DATA_RATE;
    configs[0].Value = 500000;

    configs[1].Parameter = LOOPBACK;
    configs[1].Value = 0;

    configs[2].Parameter = BIT_SAMPLE_POINT;
    configs[2].Value = 80;

    configs[3].Parameter = ISO15765_STMIN;
    configs[3].Value = 0;

    configList.NumOfParams = 4;
    configList.ConfigPtr = configs;

    status = m_j2534->PassThruIoctl(m_channelId, SET_CONFIG, (void*)&configList, nullptr);
    if (status != STATUS_NOERROR) {
        QString errorMsg = QString("Failed to set channel parameters: %1").arg(getErrorText(status));
        qDebug() << errorMsg;
        appendToLog(errorMsg);
        return false;
    }

    qDebug() << "Connected to CAN channel successfully. Channel ID:" << m_channelId;
    return true;
}

bool LoggerWindow::setupMessageFilter()
{
    appendToLog("Setting up message filter...");

    PASSTHRU_MSG maskMsg = {0}, patternMsg = {0}, flowControlMsg = {0};

    // Настройка маски
    maskMsg.ProtocolID = ISO15765;
    maskMsg.Data[0] = maskMsg.Data[1] = maskMsg.Data[2] = maskMsg.Data[3] = 0xFF;
    maskMsg.DataSize = 4;

    // Настройка шаблона для приема
    patternMsg.ProtocolID = ISO15765;
    patternMsg.Data[0] = 0x00;
    patternMsg.Data[1] = 0x00;
    patternMsg.Data[2] = 0x07;
    patternMsg.Data[3] = 0xE8;  // ID ответа от ECU
    patternMsg.DataSize = 4;

    // Настройка Flow Control
    flowControlMsg.ProtocolID = ISO15765;
    flowControlMsg.Data[0] = 0x00;
    flowControlMsg.Data[1] = 0x00;
    flowControlMsg.Data[2] = 0x07;
    flowControlMsg.Data[3] = 0xE0;  // ID запроса к ECU
    flowControlMsg.DataSize = 4;

    appendToLog("Attempting to start message filter...");

    long status = m_j2534->PassThruStartMsgFilter(
        m_channelId,
        FLOW_CONTROL_FILTER,
        &maskMsg,
        &patternMsg,
        &flowControlMsg,
        &m_msgId
        );

    if (status != STATUS_NOERROR) {
        appendToLog(QString("Failed to set message filter: %1").arg(getErrorText(status)));
        return false;
    }

    appendToLog("Message filter set successfully");

    // Очистка буферов
    m_j2534->PassThruIoctl(m_channelId, CLEAR_TX_BUFFER, nullptr, nullptr);
    m_j2534->PassThruIoctl(m_channelId, CLEAR_RX_BUFFER, nullptr, nullptr);

    return true;
}

void LoggerWindow::setupPlot()
{
    qDebug() << "Starting setupPlot";

    if (!m_plot) {
        qDebug() << "Plot widget is null!";
        return;
    }

    try {
        m_plot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);

        // Настройка легенды
        m_plot->legend->setVisible(true);
        m_plot->legend->setFont(QFont("Arial", 9));

        // Настройка осей
        m_plot->xAxis->setLabel("Time (s)");
        m_plot->yAxis->setLabel("Value");

        // Автоматическое обновление диапазона
        m_plot->setAutoAddPlottableToLegend(true);
        m_plot->axisRect()->setupFullAxesBox(true);

        qDebug() << "Plot setup successful";
    } catch (const std::exception& e) {
        qDebug() << "Exception in setupPlot:" << e.what();
    } catch (...) {
        qDebug() << "Unknown exception in setupPlot";
    }
}

void LoggerWindow::updatePlot()
{
    const int MAX_POINTS = 100; // Максимальное количество точек на графике

    // Добавляем текущее время
    double currentTime = QDateTime::currentDateTime().toMSecsSinceEpoch() / 1000.0;
    if (m_timeData.isEmpty()) {
        m_timeData.append(0);
    } else {
        m_timeData.append(currentTime - m_timeData.first());
    }

    // Ограничиваем количество точек
    if (m_timeData.size() > MAX_POINTS) {
        m_timeData.removeFirst();
        // Обновляем историю для всех параметров
        for (auto& paramList : m_parameterValues) {
            paramList.history.removeFirst();
        }
    }

    // Обновляем графики для каждого параметра
    for (auto it = m_parameterValues.begin(); it != m_parameterValues.end(); ++it) {
        QString name = it.key();
        ParameterInfo& info = it.value();

        // Находим или создаем график для параметра
        QCPGraph* graph = nullptr;
        for (int i = 0; i < m_plot->graphCount(); ++i) {
            if (m_plot->graph(i)->name() == name) {
                graph = m_plot->graph(i);
                break;
            }
        }
        if (!graph) {
            graph = m_plot->addGraph();
            graph->setName(name);
            // Используем QRandomGenerator вместо устаревшего qrand()
            graph->setPen(QPen(QColor::fromHsv(QRandomGenerator::global()->bounded(360), 255, 255)));
        }

        // Обновляем данные
        graph->setData(m_timeData, info.history);
    }

    // Масштабируем оси
    m_plot->xAxis->rescale();
    m_plot->yAxis->rescale();
    m_plot->replot();
}

void LoggerWindow::updateParameter(const QString& name, const QString& value, const QString& units)
{
    // Обновляем значение в дереве параметров
    for (int i = 0; i < ui->parametersTree->topLevelItemCount(); ++i) {
        QTreeWidgetItem* item = ui->parametersTree->topLevelItem(i);
        if (item->text(1) == name) {
            item->setText(2, value);
            item->setText(3, units);
            break;
        }
    }

    // Отправляем сигнал для обновления отдельных окон
    emit parameterUpdated(name, value, units);
}

void LoggerWindow::onParameterItemChanged(QTreeWidgetItem* item, int column)
{
    if (column != 0) // Проверяем, что изменение произошло в колонке с чекбоксом
        return;

    if (item->checkState(0) == Qt::Checked) {
        // Получаем имя параметра
        QString paramName = item->text(1);

        // Создаем и показываем новое окно для параметра
        ParameterWindow* paramWindow = new ParameterWindow(paramName, this);
        paramWindow->setAttribute(Qt::WA_DeleteOnClose); // Автоматическое удаление при закрытии

        // Подключаем обновление значения
        connect(this, &LoggerWindow::parameterUpdated,
                paramWindow, &ParameterWindow::updateValue);

        // Сохраняем указатель на окно
        m_parameterWindows[paramName] = paramWindow;

        // Показываем окно
        paramWindow->show();
    } else {
        // Если чекбокс снят, закрываем соответствующее окно
        QString paramName = item->text(1);
        if (m_parameterWindows.contains(paramName)) {
            m_parameterWindows[paramName]->close();
            m_parameterWindows.remove(paramName);
        }
    }
}

void LoggerWindow::startLogging()
{
    if (!m_readTimer) return;

    m_isLogging = true;
    setupRequestQueue();
    m_readTimer->start(100); // 100ms интервал

    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_fileLogButton->setEnabled(true);
}

void LoggerWindow::stopLogging()
{
    if (!m_readTimer) return;

    m_isLogging = false;
    m_readTimer->stop();

    m_startButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_fileLogButton->setEnabled(false);

    if (m_isFileLogging) {
        m_csvFile.close();
        m_isFileLogging = false;
    }
}

void LoggerWindow::setupLoggerDefinitionLoader()
{
    // Добавляем кнопку для загрузки файла определений в UI
    QPushButton *loadDefinitionButton = new QPushButton("Load Definition", this);
    ui->toolBar->addWidget(loadDefinitionButton);

    connect(loadDefinitionButton, &QPushButton::clicked, this, &LoggerWindow::loadLoggerDefinition);
}

void LoggerWindow::loadLoggerDefinition()
{
    QFile file("logger.xml");
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open logger definition file";
        return;
    }

    QDomDocument doc;
    if (!doc.setContent(&file)) {
        file.close();
        qDebug() << "Failed to parse XML content";
        return;
    }
    file.close();

    m_parameters.clear();

    QDomElement root = doc.documentElement();
    QDomElement categoryElement = root.firstChildElement("category");

    while (!categoryElement.isNull()) {
        QString categoryName = categoryElement.attribute("name");
        QList<ParameterDefinition>& categoryParams = m_parameters[categoryName];

        QDomElement paramElement = categoryElement.firstChildElement("parameter");
        while (!paramElement.isNull()) {
            ParameterDefinition param;
            param.category = categoryName;
            param.id = paramElement.attribute("id");
            param.name = paramElement.attribute("name");
            param.description = paramElement.attribute("description");
            param.address = paramElement.attribute("address");
            param.units = paramElement.attribute("units");
            param.expression = paramElement.attribute("expression");
            param.format = paramElement.attribute("format");

            // Парсинг min, max, step если они есть
            bool ok;
            float minVal = paramElement.attribute("min").toFloat(&ok);
            if (ok) param.min = minVal;

            float maxVal = paramElement.attribute("max").toFloat(&ok);
            if (ok) param.max = maxVal;

            float stepVal = paramElement.attribute("step").toFloat(&ok);
            if (ok) param.step = stepVal;

            // Парсинг списка единиц измерения, если они есть
            QDomElement unitsElement = paramElement.firstChildElement("units");
            while (!unitsElement.isNull()) {
                QString value = unitsElement.attribute("value");
                QString text = unitsElement.attribute("text");
                param.unitsList.append(qMakePair(value, text));
                unitsElement = unitsElement.nextSiblingElement("units");
            }

            categoryParams.append(param);

            // Добавляем информацию о параметре в m_parameterValues
            ParameterInfo paramInfo;
            paramInfo.name = param.name;
            paramInfo.units = param.units;
            paramInfo.formula = param.expression;
            m_parameterValues[param.id] = paramInfo;

            paramElement = paramElement.nextSiblingElement("parameter");
        }

        categoryElement = categoryElement.nextSiblingElement("category");
    }

    updateParametersTree();
}

void LoggerWindow::updateParametersTree()
{
    ui->parametersTree->clear();

    for (auto it = m_parameters.constBegin(); it != m_parameters.constEnd(); ++it) {
        QTreeWidgetItem* categoryItem = new QTreeWidgetItem(ui->parametersTree);
        categoryItem->setText(0, it.key());

        const QList<ParameterDefinition>& params = it.value();
        for (const auto& param : params) {
            QTreeWidgetItem* paramItem = new QTreeWidgetItem(categoryItem);
            paramItem->setText(0, param.name);
            paramItem->setText(1, param.id);
            paramItem->setText(2, ""); // Значение будет обновляться позже
            paramItem->setText(3, param.units);
            paramItem->setText(4, param.description);

            // Сохраняем ParameterDefinition в данных элемента
            paramItem->setData(0, Qt::UserRole, QVariant::fromValue(param));
        }
    }

    ui->parametersTree->expandAll();
}

void LoggerWindow::processParameterData(const QString& paramId, const QByteArray& data)
{
    const ConfigManager::Parameter* param = ConfigManager::instance().getParameter(paramId);
    if (!param || param->conversions.empty()) {
        qDebug() << "Parameter not found or has no conversions:" << paramId;
        return;
    }

    const ConfigManager::Conversion& conv = param->conversions.front();

    std::vector<double> values;
    for (char byte : data) {
        values.push_back(static_cast<unsigned char>(byte));
    }

    // Предполагаем, что у ConfigManager::Conversion есть метод evaluate
    double value = conv.evaluate(values);

    QString formattedValue;
    QString format = conv.format;
    if (!format.isEmpty()) {
        if (format.contains('f')) {
            int precision = format.mid(format.indexOf('.') + 1, 1).toInt();
            formattedValue = QString::number(value, 'f', precision);
        } else if (format.contains('d')) {
            formattedValue = QString::number(static_cast<int>(value));
        } else {
            formattedValue = QString::number(value);
        }
    } else {
        formattedValue = QString::number(value);
    }

    updateParameterValue(param->name, formattedValue, conv.units);
}

void LoggerWindow::updateParameterValue(const QString& name, const QString& value, const QString& units)
{
    // Обновление в дереве параметров
    QList<QTreeWidgetItem*> items = ui->parametersTree->findItems(name, Qt::MatchExactly | Qt::MatchRecursive, 1);
    for (QTreeWidgetItem* item : items) {
        item->setText(2, value);
        item->setText(3, units);
    }

    // Обновление виджета параметра
    if (m_parameterWidgets.contains(name)) {
        bool ok;
        double doubleValue = value.toDouble(&ok);
        if (ok) {
            m_parameterWidgets[name]->updateValue(doubleValue);
        } else {
            m_parameterWidgets[name]->updateValue(value);
        }
    }

    // Обновление данных для графика
    if (m_parameterValues.contains(name)) {
        bool ok;
        double doubleValue = value.toDouble(&ok);
        if (ok) {
            m_parameterValues[name].history.append(doubleValue);
            if (m_parameterValues[name].history.size() > MAX_HISTORY_SIZE) {
                m_parameterValues[name].history.removeFirst();
            }
        }
    }

    // Отправка сигнала
    emit parameterUpdated(name, value, units);

    // Обновление графика
    updatePlot();
}

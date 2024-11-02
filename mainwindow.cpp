#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QMessageBox>
#include <QSettings>
#include <QTimer>
#include <QThread>
#include <QFileInfo>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , sessionOpened(false)
    , m_readTimer(nullptr)
    , m_j2534(nullptr)
    , m_deviceId(0)
    , m_channelId(0)
    , m_msgId(0)
    , m_loggerWindow(nullptr)
{
    ui->setupUi(this);

    m_readTimer = new QTimer(this);
    connect(m_readTimer, &QTimer::timeout, this, &MainWindow::onReadTimer);
    connect(ui->IdentificationButton, &QPushButton::clicked, this, &MainWindow::onIdentificationButtonClicked);
    connect(ui->LoggerButton, &QPushButton::clicked, this, &MainWindow::on_LoggerButton_clicked);

    //connect(this, &MainWindow::openLoggerWindow, this, &MainWindow::onOpenLogger);

    updateDeviceList();
}

MainWindow::~MainWindow()
{
    disconnectCurrent();
    delete ui;
    if (m_loggerWindow) {
        delete m_loggerWindow;
    }
}


void MainWindow::on_LoggerButton_clicked()
{
    if (!m_loggerWindow) {
        m_loggerWindow = new LoggerWindow(this);
        connect(m_loggerWindow, &LoggerWindow::closed, this, &MainWindow::show);
    }
    m_loggerWindow->show();
    this->hide();
}

void MainWindow::show()
{
    QMainWindow::show();
    if (m_loggerWindow) {
        m_loggerWindow->hide();
    }
}

void MainWindow::updateDeviceList()
{
    ui->adapterComboBox->clear();

    QSettings settings32("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\PassThruSupport.04.04", QSettings::NativeFormat);
    QSettings settings64("HKEY_LOCAL_MACHINE\\SOFTWARE\\PassThruSupport.04.04", QSettings::NativeFormat);

    QStringList vendors;
    vendors.append(settings32.childGroups());
    vendors.append(settings64.childGroups());
    vendors.removeDuplicates();

    for (const QString &vendor : vendors) {
        settings32.beginGroup(vendor);
        QString name32 = settings32.value("Name", "").toString();
        QString dll32 = settings32.value("FunctionLibrary", "").toString();
        settings32.endGroup();

        settings64.beginGroup(vendor);
        QString name64 = settings64.value("Name", "").toString();
        QString dll64 = settings64.value("FunctionLibrary", "").toString();
        settings64.endGroup();

        QString name = name64.isEmpty() ? name32 : name64;
        QString dll = dll64.isEmpty() ? dll32 : dll64;

        if (!name.isEmpty() && !dll.isEmpty()) {
            ui->adapterComboBox->addItem(name);
            adapterToDllMap[name] = dll;
        }
    }

    if (ui->adapterComboBox->count() == 0) {
        appendToLog("No J2534 adapters found");
    }
}

void MainWindow::appendToLog(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    ui->logTextEdit->append(QString("[%1] %2").arg(timestamp).arg(message));
}

void MainWindow::disconnectCurrent()
{
    if (m_j2534) {
        if (m_msgId != 0) {
            m_j2534->PassThruStopMsgFilter(m_channelId, m_msgId);
            m_msgId = 0;
        }
        if (m_channelId != 0) {
            m_j2534->PassThruDisconnect(m_channelId);
            m_channelId = 0;
        }
        if (m_deviceId != 0) {
            m_j2534->PassThruClose(m_deviceId);
            m_deviceId = 0;
        }
        delete m_j2534;
        m_j2534 = nullptr;
    }
}

bool MainWindow::initializeAdapter()
{
    if (!m_j2534 || !m_deviceId) {
        appendToLog("Device not properly initialized");
        return false;
    }

    char apiVersion[256] = {0};
    char dllVersion[256] = {0};
    char firmwareVersion[256] = {0};

    // Используем PassThruReadVersion
    long result = m_j2534->PassThruReadVersion(apiVersion, dllVersion, firmwareVersion, m_deviceId);

    if (result != STATUS_NOERROR) {
        appendToLog("Failed to get versions: " + getErrorText(result));
        return false;
    }

    appendToLog(QString("API Version: %1").arg(apiVersion));
    appendToLog(QString("DLL Version: %1").arg(dllVersion));
    appendToLog(QString("Firmware Version: %1").arg(firmwareVersion));

    return true;
}

void MainWindow::configureAdapter()
{
    if (!m_j2534 || !m_channelId) {
        appendToLog("Error: Adapter or channel not initialized");
        return;
    }

    SCONFIG configs[3]; // Увеличиваем размер массива для NETWORK_LINE
    SCONFIG_LIST input = {0};

    configs[0].Parameter = DATA_RATE;
    configs[0].Value = 500000;  // 500 kbps

    configs[1].Parameter = LOOPBACK;
    configs[1].Value = 0;       // Disable loopback

    // Добавляем настройку NETWORK_LINE
    configs[2].Parameter = NETWORK_LINE;
    configs[2].Value = 2; // Замените на подходящее значение!

    input.NumOfParams = 3; // Теперь у нас 3 параметра
    input.ConfigPtr = configs;

    long result = m_j2534->PassThruIoctl(m_channelId, SET_CONFIG, (void*)&input, nullptr);
    if (result != STATUS_NOERROR) {
        appendToLog("Failed to configure adapter: " + getErrorText(result));

        // Вывод более подробной информации об ошибке:
        char errorText[80] = {0};
        m_j2534->PassThruGetLastError(errorText);
        appendToLog(QString("J2534 Error: %1").arg(errorText));
    } else {
        appendToLog("Adapter configured successfully");
    }
}

QString MainWindow:: getErrorText(long errorCode)
{
    if (!m_j2534) {
        return QString::number(errorCode);
    }

    char errorText[80] = {0};
    m_j2534->PassThruGetLastError(errorText);
    return QString("%1 (%2)").arg(errorCode).arg(errorText);
}

/*bool MainWindow::setupMessageFilter()
{
    if (!m_j2534 || !m_channelId) {
        return false;
    }

    PASSTHRU_MSG maskMsg = {0};
    PASSTHRU_MSG patternMsg = {0};
    unsigned long msgId = 0;

    maskMsg.ProtocolID = CAN;
    maskMsg.DataSize = 4;
    memset(maskMsg.Data, 0xFF, 4);

    patternMsg.ProtocolID = CAN;
    patternMsg.DataSize = 4;
    memset(patternMsg.Data, 0x00, 4);

    long result = m_j2534->PassThruStartMsgFilter(
        m_channelId,
        PASS_FILTER,
        &maskMsg,
        &patternMsg,
        nullptr,
        &msgId
        );

    if (result != STATUS_NOERROR) {
        appendToLog("Failed to setup message filter: " + getErrorText(result));
        return false;
    }

    return true;
}*/

bool MainWindow::sendCANMessage(const QByteArray &data)
{
    appendToLog("Preparing message to send...");

    PASSTHRU_MSG msg;
    memset(&msg, 0, sizeof(msg));

    msg.ProtocolID = CAN;
    msg.TxFlags = 0;  // Стандартный 11-битный CAN

    // Устанавливаем 11-битный ID сообщения (например, 0x7DF)
    unsigned long canId = 0x7DF;  // Пример стандартного ID

    // Записываем ID в первые 4 байта в формате, ожидаемом J2534 для 11-битного ID
    msg.Data[0] = (canId & 0xFF);
    msg.Data[1] = ((canId >> 8) & 0x07);
    msg.Data[2] = 0;
    msg.Data[3] = 0;

    // Копируем данные после ID
    if (data.size() <= 8) {  // Проверяем, что данные не превышают 8 байт
        memcpy(msg.Data + 4, data.constData(), data.size());
        msg.DataSize = data.size() + 4;  // Общий размер: ID + данные
    } else {
        appendToLog("Error: Data size exceeds 8 bytes");
        return false;
    }

    unsigned long numMsgs = 1;

    appendToLog(QString("Attempting to send message with Channel ID: %1").arg(m_channelId));

    // Выводим отладочную информацию о сообщении перед отправкой
    appendToLog(QString("Sending message: ID = 0x%1, Data = %2")
                    .arg(static_cast<unsigned int>(canId), 3, 16, QChar('0'))
                    .arg(QString(QByteArray((char*)msg.Data + 4, msg.DataSize - 4).toHex(' '))));

    long status = m_j2534->PassThruWriteMsgs(m_channelId, &msg, &numMsgs, 0);
    if (status != STATUS_NOERROR) {
        appendToLog("Error sending CAN message: " + getErrorText(status));
        return false;
    }

    appendToLog("Message sent successfully");
    appendToLog(formatCANMessage(msg, data.size()));
    return true;
}

void MainWindow::readMessages()
{
    if (!m_j2534 || !m_channelId) {
        return;
    }

    PASSTHRU_MSG msg;
    unsigned long numMsgs = 1;
    long result = m_j2534->PassThruReadMsgs(m_channelId, &msg, &numMsgs, 0);

    if (result == STATUS_NOERROR && numMsgs > 0) {
        // Получаем ID из первых 4 байт
        unsigned long id;
        memcpy(&id, msg.Data, 4);

        // Вычисляем размер данных
        int dataSize = msg.DataSize - 4;  // Общий размер минус размер ID

        QString formattedId = QString("%1").arg(id, 3, 16, QChar('0')).toUpper();

        // Форматируем данные
        QString dataHex;
        for (int i = 0; i < dataSize; ++i) {
            dataHex += QString("%1 ").arg(static_cast<unsigned char>(msg.Data[i + 4]), 2, 16, QChar('0')).toUpper();
        }
        dataHex = dataHex.trimmed();

        QString message = QString("%1\t%2\t%3")
                              .arg(formattedId)
                              .arg(dataSize)
                              .arg(dataHex);

        appendToLog("Received: " + message);
    }
}

QString MainWindow::formatCANMessage(const PASSTHRU_MSG &msg, int dataSize)
{
    // Получаем ID из первых двух байтов для 11-битного ID
    unsigned int id =
        (static_cast<unsigned int>(msg.Data[0])) |
        (static_cast<unsigned int>(msg.Data[1] & 0x07) << 8);

    // Форматируем ID в hex
    QString formattedId = QString("%1").arg(id, 3, 16, QChar('0')).toUpper();

    // Форматируем данные
    QString dataHex;
    for (int i = 0; i < dataSize; ++i) {
        dataHex += QString("%1 ").arg(static_cast<unsigned char>(msg.Data[i + 4]), 2, 16, QChar('0')).toUpper();
    }
    dataHex = dataHex.trimmed();

    return QString("%1\t%2\t%3")
        .arg(formattedId)
        .arg(dataSize)
        .arg(dataHex);
}


void MainWindow::onReadTimer()
{
    readMessages();
}

void MainWindow::onAdapterComboBoxCurrentTextChanged(const QString &currentText)
{
    disconnectCurrent();

    QString dllName = ui->adapterComboBox->currentData().toString();
    appendToLog("Attempting to initialize DLL: " + dllName);

    m_j2534 = new J2534(dllName);

    if (!m_j2534->init()) {
        appendToLog("Failed to initialize J2534 DLL: " + dllName);
        delete m_j2534;
        m_j2534 = nullptr;
        return;
    }

    appendToLog("DLL initialized successfully");

    // Открываем устройство
    appendToLog("Attempting to open device...");
    long status = m_j2534->PassThruOpen(nullptr, &m_deviceId);
    if (status != STATUS_NOERROR) {
        appendToLog("Failed to open device: " + getErrorText(status));
        delete m_j2534;
        m_j2534 = nullptr;
        return;
    }

    appendToLog("Device opened successfully. Device ID: " + QString::number(m_deviceId));

    // Инициализируем адаптер
    if (!initializeAdapter()) {
        disconnectCurrent();
        return;
    }

    // Используем метод connectToCANChannel вместо прямого вызова PassThruConnect
    if (!connectToCANChannel()) {
        disconnectCurrent();
        return;
    }

    // Начинаем чтение данных
    m_readTimer->start(10);
}

QByteArray passthruMsgToByteArray(const PASSTHRU_MSG &msg) {
    QByteArray byteArray;
    byteArray.append(reinterpret_cast<const char*>(&msg), sizeof(PASSTHRU_MSG));
    return byteArray;
}

void MainWindow::processResponse(uint8_t requestedService, const QByteArray& initialResponse) {
    appendToLog(QString("Processing response for service 0x%1").arg(requestedService, 2, 16, QChar('0')));

    if (initialResponse.size() < 2) {
        appendToLog("Invalid response: too short");
        return;
    }

    if (initialResponse.isEmpty()) {
        appendToLog("Error: Empty response received");
        return;
    }

    uint8_t responseService = static_cast<uint8_t>(initialResponse[0]);

    // Вывод полного ответа для отладки
    QString responseHex;
    for (char byte : initialResponse) {
        responseHex += QString("%1 ").arg(static_cast<uint8_t>(byte), 2, 16, QChar('0'));
    }
    appendToLog("Full response: " + responseHex.trimmed());

    // Проверяем, является ли ответ положительным (service + 0x40)
    if (responseService == requestedService + 0x40) {
        appendToLog("Positive response received");

        if (requestedService == 0x10 && initialResponse[1] == 0xC0) {
            sessionOpened = true;
            appendToLog("Diagnostic session opened successfully");
        }
        else if (requestedService == 0x21) {
            // Обработка ответов на запросы чтения данных
            if (initialResponse.size() < 3) {
                appendToLog("Response too short for service 0x21");
                return;
            }

            QByteArray data = initialResponse.mid(2);  // Пропускаем SID и PID

            // Преобразуем байты в читаемый вид
            QString hexData;
            QString asciiData;

            for (int i = 0; i < data.size(); ++i) {
                unsigned char byte = static_cast<unsigned char>(data[i]);
                hexData += QString("%1 ").arg(byte, 2, 16, QChar('0'));

                // Добавляем ASCII представление, если символ печатаемый
                if (byte >= 32 && byte <= 126) {
                    asciiData += QChar(byte);
                } else {
                    asciiData += '.';
                }
            }

            appendToLog("Data (HEX): " + hexData.trimmed());
            appendToLog("Data (ASCII): " + asciiData);

            // Сохраняем данные в соответствующее поле на основе PID
            uint8_t pid = static_cast<uint8_t>(initialResponse[1]);
            QString pidDescription;
            QString result;
            switch (pid) {
            case 0x10:
                pidDescription = "ECU Serial Number";
                m_ecuSerial = asciiData;
                result = m_ecuSerial;
                break;
            case 0xF0:
                pidDescription = "VIN Data";
                m_vinData = asciiData;
                result = m_vinData;
                break;
            case 0x83:
                pidDescription = "ECU Hardware Number";
                m_ecuHardware = asciiData;
                result = m_ecuHardware;
                break;
            case 0xA0:
                pidDescription = "Boot Software Version";
                m_bootSoftware = asciiData;
                result = m_bootSoftware;
                break;
            case 0x81:
                pidDescription = "ECU Manufacturing Info";
                m_ecuManufacturing = asciiData;
                result = m_ecuManufacturing;
                break;
            case 0xF1:
                pidDescription = "ECU Software Version";
                m_ecuSoftware = asciiData;
                result = m_ecuSoftware;
                break;
            case 0xFE:
                pidDescription = "ECU Supplier Number";
                m_ecuSupplier = asciiData;
                result = m_ecuSupplier;
                break;
            case 0xFF:
                pidDescription = "ECU Manufacturing Date";
                m_ecuManufacturingDate = asciiData;
                result = m_ecuManufacturingDate;
                break;
            default:
                pidDescription = QString("Unknown PID: 0x%1").arg(pid, 2, 16, QChar('0'));
                break;
            }
            appendToLog(QString("-----------------"));
            appendToLog(QString("PID 0x%1 - %2: %3").arg(pid, 2, 16, QChar('0')).arg(pidDescription).arg(result));
            appendToLog(QString("-----------------"));
        }
    }
    else if (responseService == 0x7F) {
        // Обработка отрицательного ответа
        if (initialResponse.size() >= 3) {
            uint8_t nrc = static_cast<uint8_t>(initialResponse[2]);
            appendToLog(QString("Negative response: NRC 0x%1").arg(nrc, 2, 16, QChar('0')));
        } else {
            appendToLog("Invalid negative response format");
        }
        if (requestedService == 0x10) {
            sessionOpened = false;
        }
    }
    else if (responseService == 0x30) {
        // Flow Control Frame
        appendToLog("Flow Control Frame received");
    }
    else {
        appendToLog(QString("Unexpected response service: 0x%1").arg(responseService, 2, 16, QChar('0')));
    }
}

void MainWindow::processCompleteResponse(uint8_t requestedService, const QByteArray& data) {
    QString logMsg = QString("Complete response for service 0x%1: ").arg(requestedService, 2, 16, QChar('0'));
    for (char byte : data) {
        logMsg += QString("%1 ").arg(static_cast<uint8_t>(byte), 2, 16, QChar('0'));
    }
    appendToLog(logMsg.trimmed());

    // Здесь можно добавить специфическую обработку для разных сервисов
    if (requestedService == 0x21) {  // ReadDataByLocalIdentifier
        QByteArray value = data.mid(2);  // Пропускаем SID и PID
        QString strValue = QString::fromLatin1(value);
        appendToLog(QString("Decoded value: %1").arg(strValue));
    }
}

QByteArray MainWindow::waitForResponse(int timeout) {
    PASSTHRU_MSG msg;
    unsigned long numMsgs = 1;
    if (!m_j2534) {
        appendToLog("Error: J2534 interface not initialized");
        return QByteArray();
    }
    long status = m_j2534->PassThruReadMsgs(m_channelId, &msg, &numMsgs, timeout);

    if (status != STATUS_NOERROR || numMsgs == 0) {
        appendToLog("Error or timeout while waiting for response");
        return QByteArray();
    }

    return QByteArray(reinterpret_cast<char*>(msg.Data), msg.DataSize);
}

QString MainWindow::getErrorDescription(uint8_t errorCode) {
    switch (errorCode) {
    case 0x10:
        return "General Reject";
    case 0x11:
        return "Service Not Supported";
    case 0x12:
        return "Sub-Function Not Supported";
    case 0x13:
        return "Invalid Format or Message Length";
    case 0x14:
        return "Response Too Long";
    case 0x21:
        return "Busy - Repeat Request";
    case 0x22:
        return "Conditions Not Correct";
    case 0x24:
        return "Request Sequence Error";
    case 0x31:
        return "Request Out Of Range";
    case 0x33:
        return "Security Access Denied";
    case 0x35:
        return "Invalid Key";
    case 0x36:
        return "Exceed Number Of Attempts";
    case 0x37:
        return "Required Time Delay Not Expired";
    case 0x70:
        return "Upload/Download Not Accepted";
    case 0x71:
        return "Transfer Data Suspended";
    case 0x72:
        return "General Programming Failure";
    case 0x73:
        return "Wrong Block Sequence Counter";
    case 0x78:
        return "Request Correctly Received - Response Pending";
    case 0x7E:
        return "Sub-Function Not Supported In Active Session";
    case 0x7F:
        return "Service Not Supported In Active Session";
    default:
        return QString("Unknown Error (0x%1)").arg(errorCode, 2, 16, QChar('0'));
    }
}


QString MainWindow::interpretResponse(uint8_t service, uint16_t pid, const QByteArray &data) {
    QString interpretation;

    switch (service) {
    case 0x10: // Diagnostic Session Control
        if (data.size() >= 2 && data[0] == 0x50) {
            interpretation = "Diagnostic Session Control: ";
            switch (data[1]) {
            case 0xC0:
                interpretation += "Extended Session Activated";
                break;
            default:
                interpretation += QString("Unknown Session Type (0x%1)")
                                      .arg(static_cast<uint8_t>(data[1]), 2, 16, QChar('0'));
            }
        }
        break;

    case 0x21: // Read Data By Local Identifier
        switch (pid) {
        case 0x10: // Serial Number
            interpretation = "ECU Serial Number: ";
            if (data.size() >= 3) {
                interpretation += QString::fromLatin1(data.mid(2)); // 13SR4A00000
            }
            break;

        case 0xF0: // VIN Related
            interpretation = "VIN Related Data: ";
            if (data.size() >= 3) {
                interpretation += QString::fromLatin1(data.mid(2)); // BV83A01A06F5KH3D8ENH
            }
            break;

        case 0x83: // Hardware Number
            interpretation = "ECU Hardware Number: ";
            if (data.size() >= 3) {
                interpretation += QString::fromLatin1(data.mid(2)); // 3SR4A0A06
            }
            break;

        case 0xA0: // Boot Software Version
            interpretation = "Boot Software Version: ";
            if (data.size() >= 3) {
                interpretation += QString::fromLatin1(data.mid(2)); // CD14D113
            }
            break;

        case 0x81: // Manufacturing Data
            interpretation = "ECU Manufacturing Info: ";
            if (data.size() >= 3) {
                interpretation += QString::fromLatin1(data.mid(2)); // JN8AF5MV3FT550004
            }
            break;

        case 0xF1: // Software Version
            interpretation = "ECU Software Version: ";
            if (data.size() >= 3) {
                interpretation += QString::fromLatin1(data.mid(2));
            }
            break;

        case 0xFE: // Supplier Number
            interpretation = "ECU Supplier Number: ";
            if (data.size() >= 3) {
                interpretation += QString::fromLatin1(data.mid(2));
            }
            break;

        case 0xFF: // Manufacturing Info
            interpretation = "Additional Manufacturing Info: ";
            if (data.size() >= 3) {
                interpretation += QString::fromLatin1(data.mid(2));
            }
            break;
        }
        break;
    }

    // Обработка мультифреймовых сообщений
    if (!interpretation.isEmpty() && data.size() > 0) {
        if (data[0] == 0x10) { // First frame
            interpretation += " (Multi-frame message start)";
        }
        else if (data[0] >= 0x20 && data[0] <= 0x2F) { // Consecutive frame
            interpretation += QString(" (Consecutive frame %1)").arg(data[0] - 0x20);
        }
    }

    return interpretation;
}

void MainWindow::onResponseReceived(const QByteArray &response)
{
    processResponse(currentRequest.service, response);

    // После обработки ответа, отправляем следующий запрос
    sendNextRequest();
}

QByteArray MainWindow::createRequest(uint8_t service, uint16_t pid)
{
    QByteArray request;
    request.append(static_cast<char>(service));

    if (service != 0x10) {  // Для всех сервисов, кроме 0x10 (SessionControl)
        request.append(static_cast<char>((pid >> 8) & 0xFF));  // Старший байт PID
        request.append(static_cast<char>(pid & 0xFF));         // Младший байт PID
    } else {
        request.append(static_cast<char>(pid & 0xFF));  // Для SessionControl PID - это один байт
    }

    return request;
}

void MainWindow::sendNextRequest()
{
    if (requestQueue.isEmpty()) {
        appendToLog("All requests completed");
        return;
    }

    currentRequest = requestQueue.dequeue();

    // Проверка, открыта ли сессия перед отправкой запроса
    if (currentRequest.service != 0x10 && !sessionOpened) {
        appendToLog("Diagnostic session not opened. Cannot send request.");
        sendNextRequest();
        return;
    }

    QByteArray request = createRequest(currentRequest.service, currentRequest.pid);

    appendToLog(QString("Processing request: %1 (Service: %2, PID: %3)")
                    .arg(currentRequest.description)
                    .arg(currentRequest.service, 2, 16, QChar('0'))
                    .arg(currentRequest.pid, 4, 16, QChar('0')));

    if (sendRequest(request)) {
        appendToLog("Request sent successfully");
    } else {
        appendToLog("Failed to send request");
        sendNextRequest();
    }
}

bool MainWindow::sendRequest(const QByteArray &request)
{
    // Здесь должна быть реализация отправки запроса через выбранный интерфейс связи
    // Например, через последовательный порт или CAN-интерфейс
    // Возвращает true, если запрос успешно отправлен, иначе false

    // Пример заглушки:
    appendToLog("Sending request: " + request.toHex());
    return true; // Предполагаем, что запрос всегда успешно отправляется
}

QString MainWindow::formatMessage(uint16_t addr, uint8_t dlc, const QByteArray &data)
{
    QString msg = QString("%1 %2").arg(addr, 3, 16, QChar('0')).arg(dlc);

    for(int i = 0; i < data.size() && i < dlc; ++i) {
        msg += QString(" %1").arg(static_cast<uint8_t>(data[i]), 2, 16, QChar('0'));
    }

    return msg.toUpper();
}

void MainWindow::addRequest(uint8_t service, uint16_t pid, const QString& description)
{
    requestQueue.enqueue(DiagRequest(service, pid, description));
}

void MainWindow::onIdentificationButtonClicked()
{
    appendToLog("Starting identification process...");

    // Проверка выбранного адаптера
    if (!validateAdapter()) {
        appendToLog("No valid adapters available");
        return;
    }

    // Инициализация J2534
    if (!initializeJ2534()) {
        appendToLog("Failed to initialize J2534");
        disconnectCurrent();
        return;
    }

    // Подключение к CAN каналу (включая настройку фильтров)
    if (!connectToCANChannel()) {
        disconnectCurrent();
        return;
    }

    // Выполнение диагностических запросов
    executeDiagnosticRequests();

    // Отключение от устройства
    disconnectCurrent();
    appendToLog("Identification process completed.");
}

bool MainWindow::setupMessageFilter()
{
    bool isScanMatic = (m_deviceId == SCANMATIC_DEVICE_ID);

    if (isScanMatic) {
        return setupScanMaticMessageFilter();
    } else {
        return setupStandardMessageFilter();
    }
}

bool MainWindow::validateAdapter()
{
    QString adapterName = ui->adapterComboBox->currentText();
    if (ui->adapterComboBox->count() == 0 || adapterName.isEmpty()) {
        appendToLog("No adapters available in the system");
        return false;
    }

    QString dllPath = adapterToDllMap.value(adapterName);
    if (dllPath.isEmpty() || !QFileInfo(dllPath).exists()) {
        appendToLog("DLL file not found for adapter: " + adapterName);
        return false;
    }

    return true;
}

bool MainWindow::initializeJ2534()
{
    QString adapterName = ui->adapterComboBox->currentText();
    QString dllPath = adapterToDllMap.value(adapterName);

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

    appendToLog(QString("Device ID: 0x%1").arg(m_deviceId, 8, 16, QChar('0')));

    return initializeAdapter();
}

bool MainWindow::configureChannel(unsigned long baudrate, bool isScanMatic)
{
    SCONFIG configs[3];
    SCONFIG_LIST input = {0};

    configs[0].Parameter = DATA_RATE;
    configs[0].Value = baudrate;

    configs[1].Parameter = LOOPBACK;
    configs[1].Value = 0;  // Отключаем loopback

    input.NumOfParams = 2;

    if (isScanMatic) {
        configs[2].Parameter = NETWORK_LINE;
        configs[2].Value = 2;  // Специфичное значение для ScanMatic
        input.NumOfParams = 3;
    }

    input.ConfigPtr = configs;

    long status = m_j2534->PassThruIoctl(m_channelId, SET_CONFIG, (void*)&input, nullptr);
    if (status != STATUS_NOERROR) {
        appendToLog("Failed to configure channel: " + getErrorText(status));
        return false;
    }

    // Очистка буферов
    m_j2534->PassThruIoctl(m_channelId, CLEAR_TX_BUFFER, nullptr, nullptr);
    m_j2534->PassThruIoctl(m_channelId, CLEAR_RX_BUFFER, nullptr, nullptr);

    appendToLog("Channel configured successfully");
    return true;
}

bool MainWindow::initializeChannel()
{
    // Очистка буферов перед подключением
    if (m_channelId != 0) {
        m_j2534->PassThruIoctl(m_channelId, CLEAR_TX_BUFFER, nullptr, nullptr);
        m_j2534->PassThruIoctl(m_channelId, CLEAR_RX_BUFFER, nullptr, nullptr);
        m_j2534->PassThruIoctl(m_channelId, CLEAR_MSG_FILTERS, nullptr, nullptr);
        m_j2534->PassThruIoctl(m_channelId, CLEAR_PERIODIC_MSGS, nullptr, nullptr);
    }
    return true;
}

bool MainWindow::connectToCANChannel()
{
    appendToLog("Connecting to CAN channel...");

    unsigned long protocol = ISO15765;
    unsigned long flags = CAN_ID_BOTH;
    unsigned long baudrate = 500000;  // 500 kbps

    bool isScanMatic = (m_deviceId == SCANMATIC_DEVICE_ID);
    if (isScanMatic) {
        appendToLog("ScanMatic device detected - using specific settings");
        flags = CAN_ID_BOTH;  // Специфичный флаг для ScanMatic
    }

    appendToLog(QString("Connection parameters - Protocol: 0x%1, Flags: 0x%2, Baudrate: %3")
                    .arg(protocol, 8, 16, QChar('0'))
                    .arg(flags, 8, 16, QChar('0'))
                    .arg(baudrate));

    // Очистка предыдущих подключений
    if (m_channelId != 0) {
        m_j2534->PassThruDisconnect(m_channelId);
        m_channelId = 0;
    }

    // Подключение к каналу
    long status = m_j2534->PassThruConnect(m_deviceId, protocol, flags, baudrate, &m_channelId);
    if (status != STATUS_NOERROR) {
        appendToLog("Failed to connect to channel: " + getErrorText(status));
        return false;
    }

    appendToLog("Connected to CAN channel. Channel ID: " + QString::number(m_channelId));

    // Конфигурация канала
    if (!configureChannel(baudrate, isScanMatic)) {
        return false;
    }

    // Установка фильтров сообщений
    if (isScanMatic) {
        if (!setupScanMaticMessageFilter()) {
            return false;
        }
    } else {
        if (!setupStandardMessageFilter()) {
            return false;
        }
    }

    appendToLog("CAN channel setup completed successfully");
    return true;
}

bool MainWindow::setupScanMaticMessageFilter()
{
    appendToLog("Setting up ScanMatic message filter...");

    PASSTHRU_MSG maskMsg = {0}, patternMsg = {0}, flowControlMsg = {0};

    // Настройка маски для ScanMatic
    maskMsg.ProtocolID = ISO15765;
    maskMsg.Data[0] = 0xFF;
    maskMsg.Data[1] = 0xFF;
    maskMsg.Data[2] = 0xFF;
    maskMsg.Data[3] = 0xFF;
    maskMsg.DataSize = 4;

    // Настройка шаблона для приема (0x7E8)
    patternMsg.ProtocolID = ISO15765;
    patternMsg.Data[0] = 0x00;
    patternMsg.Data[1] = 0x00;
    patternMsg.Data[2] = 0x07;
    patternMsg.Data[3] = 0xE8;
    patternMsg.DataSize = 4;

    // Настройка Flow Control (0x7E0)
    flowControlMsg.ProtocolID = ISO15765;
    flowControlMsg.Data[0] = 0x00;
    flowControlMsg.Data[1] = 0x00;
    flowControlMsg.Data[2] = 0x07;
    flowControlMsg.Data[3] = 0xE0;
    flowControlMsg.DataSize = 4;

    long status = m_j2534->PassThruStartMsgFilter(
        m_channelId,
        FLOW_CONTROL_FILTER,
        &maskMsg,
        &patternMsg,
        &flowControlMsg,
        &m_msgId
        );

    if (status != STATUS_NOERROR) {
        appendToLog("Failed to set ScanMatic message filter: " + getErrorText(status));
        return false;
    }

    appendToLog("ScanMatic message filter set successfully");
    return true;
}

bool MainWindow::setupStandardMessageFilter()
{
    appendToLog("Setting up standard message filter...");

    PASSTHRU_MSG maskMsg = {0}, patternMsg = {0};

    maskMsg.ProtocolID = ISO15765;
    maskMsg.Data[0] = 0xFF;
    maskMsg.Data[1] = 0xFF;
    maskMsg.Data[2] = 0xFF;
    maskMsg.Data[3] = 0xFF;
    maskMsg.DataSize = 4;

    patternMsg.ProtocolID = ISO15765;
    patternMsg.Data[0] = 0x00;
    patternMsg.Data[1] = 0x00;
    patternMsg.Data[2] = 0x07;
    patternMsg.Data[3] = 0xE8;
    patternMsg.DataSize = 4;

    long status = m_j2534->PassThruStartMsgFilter(
        m_channelId,
        PASS_FILTER,
        &maskMsg,
        &patternMsg,
        nullptr,
        &m_msgId
        );

    if (status != STATUS_NOERROR) {
        appendToLog("Failed to set standard message filter: " + getErrorText(status));
        return false;
    }

    appendToLog("Standard message filter set successfully");
    return true;
}

bool MainWindow::sendDiagnosticRequest(uint8_t service, uint16_t pid)
{
    if (!m_j2534 || !m_channelId) {
        appendToLog("Error: Device not properly initialized");
        return false;
    }

    // Очистка буферов перед отправкой
    m_j2534->PassThruIoctl(m_channelId, CLEAR_TX_BUFFER, nullptr, nullptr);
    m_j2534->PassThruIoctl(m_channelId, CLEAR_RX_BUFFER, nullptr, nullptr);

    PASSTHRU_MSG msg = {0};
    msg.ProtocolID = ISO15765;
    msg.TxFlags = ISO15765_FRAME_PAD;

    // Установка CAN ID (0x7E0 для ScanMatic 2)
    msg.Data[0] = 0x00;
    msg.Data[1] = 0x00;
    msg.Data[2] = 0x07;
    msg.Data[3] = 0xE0;

    // Формирование данных
    msg.Data[4] = service;

    if (service == 0x10) {
        msg.Data[5] = pid & 0xFF;
        msg.DataSize = 6;
    } else {
        msg.Data[5] = pid & 0xFF;
        msg.DataSize = 6;
    }

    // Отправка сообщения
    unsigned long numMsgs = 1;
    long status = m_j2534->PassThruWriteMsgs(m_channelId, &msg, &numMsgs, 1000); // таймаут 1000 мс

    if (status != STATUS_NOERROR) {
        appendToLog("Error sending message: " + getErrorText(status));
        return false;
    }

    // Добавляем задержку между сообщениями
    QThread::msleep(50);

    return true;
}

bool MainWindow::checkAndOpenSession() {
    if (sessionOpened) {
        appendToLog("Diagnostic session already open");
        return true;
    }
    return openDiagnosticSession();
}

bool MainWindow::openDiagnosticSession() {
    appendToLog("Attempting to open diagnostic session...");

    if (sessionOpened) {
        appendToLog("Diagnostic session already opened");
        return true;
    }

    // Отправка запроса на открытие сессии
    if (!sendDiagnosticRequest(0x10, 0xC0)) {
        appendToLog("Failed to send session control request");
        return false;
    }

    // Чтение ответа
    QByteArray response = readDiagnosticResponse();
    if (response.isEmpty()) {
        appendToLog("No response received for session control request");
        return false;
    }

    // Проверка ответа
    if (response.length() >= 2 &&
        static_cast<uint8_t>(response[0]) == 0x50 &&
        static_cast<uint8_t>(response[1]) == 0xC0) {
        sessionOpened = true;
        appendToLog("Diagnostic session opened successfully");
        return true;
    }

    appendToLog("Invalid response for session control request");
    return false;
}

void MainWindow::executeDiagnosticRequests() {
    struct DiagRequest {
        uint8_t service;
        uint16_t pid;
        QString description;
        bool requiresSession;
    };

    QVector<DiagRequest> requests = {
        {0x10, 0xC0, "Open Diagnostic Session", false},
        {0x21, 0x10, "ECU Serial Number", true},        // 13SR4A00000
        {0x21, 0xF0, "VIN Related Data", true},         // BV83A01A06F5KH3D8ENH
        {0x21, 0x83, "ECU Hardware Number", true},      // 3SR4A0A06
        {0x21, 0xA0, "Boot Software Version", true},    // CD14D113
        {0x21, 0x81, "ECU Manufacturing Info", true},   // JN8AF5MV3FT550004
        {0x21, 0xF1, "ECU Software Version", true},     // Software version info
        {0x21, 0xFE, "ECU Supplier Number", true},      // Supplier information
        {0x21, 0xFF, "ECU Manufacturing Info", true}    // Additional manufacturing info
    };

    sessionOpened = false;  // Сброс флага при начале новой серии запросов

    for (const auto& req : requests) {
        appendToLog(QString("\nProcessing request: %1 (Service: 0x%2, PID: 0x%3)")
                        .arg(req.description)
                        .arg(req.service, 2, 16, QChar('0'))
                        .arg(req.pid, 4, 16, QChar('0')));

        // Если это запрос открытия сессии, обрабатываем его особым образом
        if (req.service == 0x10 && req.pid == 0xC0) {
            if (!openDiagnosticSession()) {
                appendToLog("Failed to open diagnostic session");
                break;
            }
            continue;  // Переходим к следующему запросу
        }

        // Для всех остальных запросов проверяем наличие открытой сессии
        if (!sessionOpened) {
            appendToLog("Cannot continue without diagnostic session");
            break;
        }

        // Выполнение запроса
        if (sendDiagnosticRequest(req.service, req.pid)) {
            QByteArray response = readDiagnosticResponse();
            if (!response.isEmpty()) {
                processResponse(req.service, response);

                // Добавляем паузу после успешного запроса
                QThread::msleep(100);
            } else {
                appendToLog("No response received");
            }
        } else {
            appendToLog("Failed to send diagnostic request");
        }

        // Пауза между запросами
        QThread::msleep(100);

        // Для многокадровых ответов добавляем дополнительную задержку
        if (req.service == 0x21) {
            QThread::msleep(100);
        }
    }

    appendToLog("\nDiagnostic sequence completed");
}


QByteArray MainWindow::readDiagnosticResponse()
{
    QByteArray response;
    QElapsedTimer timer;
    timer.start();
    int timeoutMs = 3000; // 3 секунды таймаут

    while (timer.elapsed() < timeoutMs) {
        PASSTHRU_MSG msg = {0};
        unsigned long numMsgs = 1;

        long status = m_j2534->PassThruReadMsgs(m_channelId, &msg, &numMsgs, 100);

        if (status != STATUS_NOERROR) {
            if (status != ERR_TIMEOUT && status != ERR_BUFFER_EMPTY) {
                appendToLog("Read error: " + getErrorText(status));
            }
            QThread::msleep(10);
            continue;
        }

        if (numMsgs > 0) {
            // Проверяем, что это ответ от ЭБУ (0x7E8)
            if (msg.DataSize >= 4 &&
                msg.Data[0] == 0x00 &&
                msg.Data[1] == 0x00 &&  // CAN ID
                msg.Data[2] == 0x07 &&
                msg.Data[3] == 0xE8) {
                // Копируем данные ответа, пропуская заголовок CAN
                if (msg.DataSize >= 4) {
                    response = QByteArray((char*)&msg.Data[4], msg.DataSize - 4);
                } else {
                    appendToLog("Error: Received message too short");
                }
                appendToLog("Valid response received: " + QString(response.toHex(' ')));
                break;  // Выходим из цикла, так как получили валидный ответ
            }
        }

        QThread::msleep(10);
    }

    if (response.isEmpty()) {
        appendToLog("No valid response received within timeout");
    }

    return response;
}

void MainWindow::sendFlowControl() {
    PASSTHRU_MSG msg = {0};
    msg.ProtocolID = ISO15765;
    msg.TxFlags = ISO15765_FRAME_PAD;

    msg.Data[0] = 0x00;  // Flow Control Frame
    msg.Data[1] = 0x00;  // Block Size = 0 (непрерывная передача)
    msg.Data[2] = 0x00;  // Separation Time = 0
    msg.DataSize = 3;

    unsigned long numMsgs = 1;
    m_j2534->PassThruWriteMsgs(m_channelId, &msg, &numMsgs, 100);
}

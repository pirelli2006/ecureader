#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include "j2534.h"
#include <windows.h>
#include <QMap>
#include <QQueue>
#include "loggerwindow.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

namespace DiagConstants {
const uint32_t ECU_ADDR = 0x7E0;
const uint32_t RESPONSE_ADDR = 0x7E8;
const int TIMEOUT_MS = 3000;
const int INTER_FRAME_DELAY_MS = 100;

}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void show();

private:
    // Методы инициализации и настройки
    void updateDeviceList();
    void appendToLog(const QString &message);
    void disconnectCurrent();
    bool initializeAdapter();
    void configureAdapter();
    QString formatMessage(uint16_t addr, uint8_t dlc, const QByteArray &data);
    void sendNextRequest();
    void addRequest(uint8_t service, uint16_t pid, const QString& description);
    QByteArray createRequest(uint8_t service, uint16_t pid);
    bool sendRequest(const QByteArray &request);
    bool connectToCANChannel();
    void executeDiagnosticRequests();
    bool sendDiagnosticRequest(uint8_t service, uint16_t pid);
    bool initializeJ2534();
    bool validateAdapter();
    QByteArray readDiagnosticResponse();
    void sendFlowControl();
    bool openDiagnosticSession();
    bool sessionOpened;
    bool checkAndOpenSession();
    QString interpretResponse(uint8_t service, uint16_t pid, const QByteArray &data);
    QString getErrorDescription(uint8_t errorCode);
    void processResponse(uint8_t requestedService, const QByteArray& initialResponse);
    QByteArray waitForResponse(int timeout=1000);
    void processCompleteResponse(uint8_t requestedService, const QByteArray& data);
    bool initializeChannel();
    bool configureStandardDevice();
    bool configureChannel(unsigned long baudrate, bool isScanMatic);
    bool setupScanMaticMessageFilter();
    bool setupStandardMessageFilter();
    void loadSettings();
    void saveSettings();




    // Методы работы с сообщениями
    QString getErrorText(long errorCode);
    bool setupMessageFilter();
    bool sendCANMessage(const QByteArray &data);
    void readMessages();
    QString formatCANMessage(const PASSTHRU_MSG &msg, int dataSize);

    // Структура для хранения информации о диагностическом запросе
    struct DiagRequest {
        uint8_t service;
        uint16_t pid;
        QString description;

        DiagRequest(uint8_t s = 0, uint16_t p = 0, const QString& desc = "")
            : service(s), pid(p), description(desc) {}
    };

private slots:
    void onReadTimer();
    void onAdapterComboBoxCurrentTextChanged(const QString &currentText);
    void onIdentificationButtonClicked();
    void onResponseReceived(const QByteArray &response);
    void on_LoggerButton_clicked();


private:
    Ui::MainWindow *ui;
    QTimer *m_readTimer;
    J2534 *m_j2534;
    unsigned long m_deviceId;
    unsigned long m_channelId;
    unsigned long m_msgId;
    QMap<QString, QString> adapterToDllMap;
    DiagRequest currentRequest;  // Текущий запрос
    QQueue<DiagRequest> requestQueue;  // Очередь запросов
    QString m_ecuSerial;
    QString m_vinData;
    QString m_ecuHardware;
    QString m_bootSoftware;
    QString m_ecuManufacturing;
    QString m_ecuSoftware;
    QString m_ecuSupplier;
    QString m_ecuManufacturingDate;
    LoggerWindow* m_loggerWindow;



};

#endif // MAINWINDOW_H

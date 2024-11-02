// ecureader.h
#ifndef ECUREADER_H
#define ECUREADER_H

#include <QObject>
#include <QString>
#include <QTimer>
#include "j2534.h"
#include "device.h"

class ECUReader : public QObject {
    Q_OBJECT

public:
    explicit ECUReader(QObject *parent = nullptr);
    ~ECUReader();

    bool initialize();
    void readEcuId();

signals:
    void ecuIdRead(const QString &ecuId);
    void errorOccurred(const QString &error);

private:
    std::unique_ptr<lt::J2534> j2534;
    std::unique_ptr<lt::J2534Device> device;
    std::unique_ptr<lt::J2534Channel> channel;

    static constexpr uint32_t NISSAN_BAUDRATE = 500000;
    static constexpr uint32_t READ_TIMEOUT = 1000;

    bool setupChannel();
    QString parseEcuResponse(const std::vector<uint8_t> &response);
};

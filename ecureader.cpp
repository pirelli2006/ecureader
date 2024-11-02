// ecureader.cpp
#include "ecureader.h"

ECUReader::ECUReader(QObject *parent)
    : QObject(parent) {
}

ECUReader::~ECUReader() {
    if (channel) {
        channel->stop();
    }
}

bool ECUReader::initialize() {
    try {
        j2534 = std::make_unique<lt::J2534>("OP20PT32"); // Используйте соответствующий драйвер
        device = j2534->open();

        if (!device) {
            emit errorOccurred("Failed to open J2534 device");
            return false;
        }

        return setupChannel();
    }
    catch (const std::exception &e) {
        emit errorOccurred(QString("Initialization error: %1").arg(e.what()));
        return false;
    }
}

bool ECUReader::setupChannel() {
    try {
        // Создаем ISO14230 канал
        channel = device->connectISO14230(NISSAN_BAUDRATE);

        if (!channel) {
            emit errorOccurred("Failed to create channel");
            return false;
        }

        // Настраиваем фильтры
        lt::MessageFilter filter;
        filter.mask = 0xFFFF;
        filter.pattern = 0x0000;
        filter.flowControl = false;

        channel->startMsgFilter(lt::PASS_FILTER, filter);
        return true;
    }
    catch (const std::exception &e) {
        emit errorOccurred(QString("Channel setup error: %1").arg(e.what()));
        return false;
    }
}

void ECUReader::readEcuId() {
    try {
        if (!channel) {
            emit errorOccurred("Channel not initialized");
            return;
        }

        // Команда для запроса ECU ID (пример)
        std::vector<uint8_t> request = {0x1A, 0x90};

        channel->send(request);

        // Читаем ответ
        auto response = channel->recv(READ_TIMEOUT);

        if (response.empty()) {
            emit errorOccurred("No response from ECU");
            return;
        }

        QString ecuId = parseEcuResponse(response);
        emit ecuIdRead(ecuId);
    }
    catch (const std::exception &e) {
        emit errorOccurred(QString("Read error: %1").arg(e.what()));
    }
}

QString ECUReader::parseEcuResponse(const std::vector<uint8_t> &response) {
    // Парсинг ответа (зависит от конкретной модели ECU)
    QString result;
    for (uint8_t byte : response) {
        result += QString("%1").arg(byte, 2, 16, QChar('0')).toUpper();
    }
    return result;
}

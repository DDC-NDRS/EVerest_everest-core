// SPDX-License-Identifier: Apache-2.0
// Copyright 2020 - 2021 Pionix GmbH and Contributors to EVerest
// Portions (c) 2025 Analog Devices Inc.
#ifndef AD_ACEVSE22KWZ_KIT_SERIAL_HPP
#define AD_ACEVSE22KWZ_KIT_SERIAL_HPP

#include "adkit.pb.h"
#include <date/date.h>
#include <date/tz.h>
#include <sigslot/signal.hpp>
#include <stdint.h>
#include <termios.h>
#include <utils/thread.hpp>

class evSerial {

public:
    evSerial();
    ~evSerial();

    bool openDevice(const char* device, int baud);
    bool is_open() {
        return fd > 0;
    };

    void readThread();
    void run();

    bool reset(const std::string& reset_chip, const int reset_line);
    void keepAlive();

    void setPWM(uint32_t dc);
    void allowPowerOn(bool p);

    sigslot::signal<KeepAliveLo> signalKeepAliveLo;
    sigslot::signal<PowerMeter> signalPowerMeter;
    sigslot::signal<CpState> signalCPState;
    sigslot::signal<PpState> signalPPState;
    sigslot::signal<ErrorFlags> signalErrorFlags;
    sigslot::signal<bool> signalRelaisState;

    sigslot::signal<> signalSpuriousReset;
    sigslot::signal<> signalConnectionTimeout;

private:
    // Serial interface
    bool setSerialAttributes();
    int fd;
    int baud;

    // COBS de-/encoder
    void cobsDecodeReset();
    void handlePacket(uint8_t* buf, int len);
    void cobsDecode(uint8_t* buf, int len);
    void cobsDecodeByte(uint8_t byte);
    size_t cobsEncode(const void* data, size_t length, uint8_t* buffer);
    uint8_t msg[2048];
    uint8_t code;
    uint8_t block;
    uint8_t* decode;
    uint32_t crc32(uint8_t* buf, int len);

    // Read thread for serial port
    Everest::Thread readThreadHandle;
    Everest::Thread timeoutDetectionThreadHandle;

    bool linkWrite(EverestToMcu* m);
    volatile bool reset_done_flag;
    volatile bool forced_reset;

    bool serial_timed_out();
    void timeoutDetectionThread();
    std::chrono::time_point<date::utc_clock> last_timeout_detection_timestamp;
};

#endif

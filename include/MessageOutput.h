#pragma once

#include <HardwareSerial.h>
#include <Stream.h>
#include <mutex>
#include <ESPAsyncWebServer.h>
#include <AsyncWebSocket.h>

#define BUFFER_SIZE 500

class MessageOutputClass : public Print {
public:
    void loop();
    size_t write(uint8_t c) override;
    size_t write(const uint8_t *buffer, size_t size) override;
    String get_millis_as_String (const char *fmt = NULL);
    void logf(const char *fmt...);
    void setWebsocket(AsyncWebSocket *const pws);

private:
    char* _ws = NULL;
    char _buffer[BUFFER_SIZE];
    uint16_t _buff_pos = 0;
    uint32_t _lastSend = 0;
    bool _forceSend = false;
    AsyncWebSocket *pws = NULL;

    std::mutex _msgLock;
};

extern MessageOutputClass MessageOutput;
#pragma once

#include <WiFi.h>
#include <vector>

enum class network_event {
    NETWORK_UNKNOWN,
    NETWORK_START,
    NETWORK_STOP,
    NETWORK_CONNECTED,
    NETWORK_DISCONNECTED,
    NETWORK_GOT_IP,
    NETWORK_LOST_IP,
    NETWORK_EVENT_MAX
};

typedef std::function<void(network_event event)> NetworkEventCb;

typedef struct NetworkEventCbList {
    NetworkEventCb cb;
    network_event event;

    NetworkEventCbList()
        : cb(NULL)
        , event(network_event::NETWORK_UNKNOWN)
    {
    }
} NetworkEventCbList_t;

class NetworkSettingsClass {
public:
    NetworkSettingsClass();
    void init();
    void loop();

    IPAddress localIP();
    IPAddress gatewayIP();
    IPAddress dnsIP(uint8_t dns_no = 0);
    String macAddress();
    bool isConnected();

    bool onEvent(NetworkEventCb cbEvent, network_event event = network_event::NETWORK_EVENT_MAX);
    void raiseEvent(network_event event);

private:
    void NetworkEvent(WiFiEvent_t event);
    int connectTimeoutTimer = 0;
    int connectRedoTimer = 0;
    uint32_t lastTimerCall = 0;
    std::vector<NetworkEventCbList_t> _cbEventList;
};

extern NetworkSettingsClass NetworkSettings;
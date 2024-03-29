#include "NetworkSettings.h"
#include "MessageOutput.h"
#include <ETH.h>

NetworkSettingsClass::NetworkSettingsClass()
{
}

void NetworkSettingsClass::init()
{
    using std::placeholders::_1;

    WiFi.setScanMethod(WIFI_ALL_CHANNEL_SCAN);
    WiFi.setSortMethod(WIFI_CONNECT_AP_BY_SIGNAL);

    WiFi.onEvent(std::bind(&NetworkSettingsClass::NetworkEvent, this, _1));
    MessageOutput.logf("calling WiFi.mode");
    WiFi.mode(WIFI_STA);
    MessageOutput.logf("calling WiFi.begin");
    WiFi.begin();
}

void NetworkSettingsClass::NetworkEvent(WiFiEvent_t event)
{
    switch (event) {
    case ARDUINO_EVENT_WIFI_READY:
        MessageOutput.logf("WiFi event ARDUINO_EVENT_WIFI_READY");
        break;
    case ARDUINO_EVENT_WIFI_STA_START:
        MessageOutput.logf("WiFi event ARDUINO_EVENT_WIFI_STA_START");
        break;
    case ARDUINO_EVENT_WIFI_STA_CONNECTED:
        MessageOutput.logf("WiFi connected");
        raiseEvent(network_event::NETWORK_CONNECTED);
        break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
        MessageOutput.logf("WiFi disconnected");
        MessageOutput.logf("Try reconnecting");
        WiFi.reconnect();
        raiseEvent(network_event::NETWORK_DISCONNECTED);
        break;
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
        MessageOutput.logf("WiFi got ip: %s", WiFi.localIP().toString().c_str());
        raiseEvent(network_event::NETWORK_GOT_IP);
        break;
    default:
        MessageOutput.logf("Unknown WiFi Event: %d", event);
        break;
    }
}

bool NetworkSettingsClass::onEvent(NetworkEventCb cbEvent, network_event event)
{
    if (!cbEvent) {
        return pdFALSE;
    }
    NetworkEventCbList_t newEventHandler;
    newEventHandler.cb = cbEvent;
    newEventHandler.event = event;
    _cbEventList.push_back(newEventHandler);
    return true;
}

void NetworkSettingsClass::raiseEvent(network_event event)
{
    for (uint32_t i = 0; i < _cbEventList.size(); i++) {
        NetworkEventCbList_t entry = _cbEventList[i];
        if (entry.cb) {
            if (entry.event == event || entry.event == network_event::NETWORK_EVENT_MAX) {
                entry.cb(event);
            }
        }
    }
}


void NetworkSettingsClass::loop()
{

    if (millis() - lastTimerCall > 1000) {
        connectTimeoutTimer++;
        connectRedoTimer++;
        lastTimerCall = millis();
    }
}

IPAddress NetworkSettingsClass::localIP()
{
    return WiFi.localIP();
}

IPAddress NetworkSettingsClass::gatewayIP()
{
    return WiFi.gatewayIP();
}

IPAddress NetworkSettingsClass::dnsIP(uint8_t dns_no)
{
    return WiFi.dnsIP(dns_no);
}

String NetworkSettingsClass::macAddress()
{
    return WiFi.macAddress();
}


bool NetworkSettingsClass::isConnected()
{
    return WiFi.localIP()[0] != 0;
}


NetworkSettingsClass NetworkSettings;
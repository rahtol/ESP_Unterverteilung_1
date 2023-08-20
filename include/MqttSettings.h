// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include "NetworkSettings.h"
#include <Ticker.h>
#include <espMqttClient.h>
#include <mutex>

class MqttSettingsClass {
public:
    typedef std::function<void(String topic, String payload)> OnMessageCallback;

    MqttSettingsClass();
    void init();
    void loop();

    bool isConnected();
    void publish(const String& subtopic, const String& payload);
    void subscribe (const String &subtopic);
    void onMessageCallback (OnMessageCallback callback);

    String getPrefix();

private:
    typedef enum MqttState_t {
        mqtt_idle,
        mqtt_initializing,
        mqtt_ready,
        mqtt_wait_for_reconnect
    } MqttState;

    MqttState _mqttState;
    unsigned long t_last_check_for_prerequisites;
    unsigned long t_wait_until_reconnect;

    void onMqttDisconnect(espMqttClientTypes::DisconnectReason reason);
    void onMqttConnect(bool sessionPresent);
    void onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total);

    void createMqttClientObject();
    void performConnect();

    MqttClient* mqttClient = nullptr;
    String clientId;
    String willTopic;
    std::mutex _clientLock;
    OnMessageCallback _onMessageCallback;
};

extern MqttSettingsClass MqttSettings;
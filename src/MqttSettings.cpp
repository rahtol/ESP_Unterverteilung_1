#include "NtpSettings.h"
#include "MqttSettings.h"
#include "MessageOutput.h"

MqttSettingsClass::MqttSettingsClass()
{
}

void MqttSettingsClass::NetworkEvent(network_event event)
{
    switch (event) {
    case network_event::NETWORK_GOT_IP:
        MessageOutput.println("Network connected");
        performConnect();
        break;
    case network_event::NETWORK_DISCONNECTED:
        MessageOutput.println("Network lost connection");
        mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
        break;
    default:
        break;
    }
}

void MqttSettingsClass::onMqttConnect(bool sessionPresent)
{
    String t0 = NtpSettings.getLocalTimeAndDate();

    MessageOutput.printf("Connected to MQTT at %s.\r\n", t0.c_str());
    publish(willTopic, "online since " + t0);
}

void MqttSettingsClass::onMqttDisconnect(espMqttClientTypes::DisconnectReason reason)
{
    MessageOutput.println("Disconnected from MQTT.");

    MessageOutput.print("Disconnect reason:");
    switch (reason) {
    case espMqttClientTypes::DisconnectReason::TCP_DISCONNECTED:
        MessageOutput.println("TCP_DISCONNECTED");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
        MessageOutput.println("MQTT_UNACCEPTABLE_PROTOCOL_VERSION");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_IDENTIFIER_REJECTED:
        MessageOutput.println("MQTT_IDENTIFIER_REJECTED");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_SERVER_UNAVAILABLE:
        MessageOutput.println("MQTT_SERVER_UNAVAILABLE");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_MALFORMED_CREDENTIALS:
        MessageOutput.println("MQTT_MALFORMED_CREDENTIALS");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_NOT_AUTHORIZED:
        MessageOutput.println("MQTT_NOT_AUTHORIZED");
        break;
    default:
        MessageOutput.println("Unknown");
    }
    mqttReconnectTimer.once(
        2, +[](MqttSettingsClass* instance) { instance->performConnect(); }, this);
}

void MqttSettingsClass::onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total)
{
    MessageOutput.print("Received MQTT message on topic: ");
    MessageOutput.println(topic);

}

void MqttSettingsClass::performConnect()
{
    if (NetworkSettings.isConnected()) {
        using std::placeholders::_1;
        using std::placeholders::_2;
        using std::placeholders::_3;
        using std::placeholders::_4;
        using std::placeholders::_5;
        using std::placeholders::_6;

        std::lock_guard<std::mutex> lock(_clientLock);
        if (mqttClient == nullptr) {
            return;
        }

        MessageOutput.println("Connecting to MQTT...");
        static_cast<espMqttClient*>(mqttClient)->setServer("nas-2", 1883);
        static_cast<espMqttClient*>(mqttClient)->setCredentials("", "");
        static_cast<espMqttClient*>(mqttClient)->setWill(willTopic.c_str(), 2, true, "offline");
        static_cast<espMqttClient*>(mqttClient)->setClientId(clientId.c_str());
        static_cast<espMqttClient*>(mqttClient)->onConnect(std::bind(&MqttSettingsClass::onMqttConnect, this, _1));
        static_cast<espMqttClient*>(mqttClient)->onDisconnect(std::bind(&MqttSettingsClass::onMqttDisconnect, this, _1));
        static_cast<espMqttClient*>(mqttClient)->onMessage(std::bind(&MqttSettingsClass::onMqttMessage, this, _1, _2, _3, _4, _5, _6));
        mqttClient->connect();
    }
}

void MqttSettingsClass::performDisconnect()
{
//    const CONFIG_T& config = Configuration.get();
//    publish(config.Mqtt_LwtTopic, config.Mqtt_LwtValue_Offline);
    std::lock_guard<std::mutex> lock(_clientLock);
    if (mqttClient == nullptr) {
        return;
    }
    mqttClient->disconnect();
}

void MqttSettingsClass::performReconnect()
{
    performDisconnect();

    createMqttClientObject();

    mqttReconnectTimer.once(
        2, +[](MqttSettingsClass* instance) { instance->performConnect(); }, this);
}

bool MqttSettingsClass::getConnected()
{
    std::lock_guard<std::mutex> lock(_clientLock);
    if (mqttClient == nullptr) {
        return false;
    }
    return mqttClient->connected();
}

String MqttSettingsClass::getPrefix()
{
    return "unterverteilung/";
}

void MqttSettingsClass::publish(const String& subtopic, const String& payload)
{
    std::lock_guard<std::mutex> lock(_clientLock);
    if (mqttClient == nullptr) {
        return;
    }

    String topic = getPrefix();
    topic += subtopic;

    String value = payload;
    value.trim();

    mqttClient->publish(topic.c_str(), 0, true, value.c_str());
}

void MqttSettingsClass::init()
{
    clientId = "esp32-CD6B3C";
    willTopic = "status";

    using std::placeholders::_1;
    NetworkSettings.onEvent(std::bind(&MqttSettingsClass::NetworkEvent, this, _1));

    createMqttClientObject();
}

void MqttSettingsClass::createMqttClientObject()
{
    std::lock_guard<std::mutex> lock(_clientLock);
    if (mqttClient != nullptr) {
        delete mqttClient;
        mqttClient = nullptr;
    }

    mqttClient = static_cast<MqttClient*>(new espMqttClient);
}

MqttSettingsClass MqttSettings;
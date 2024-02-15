#include "NtpSettings.h"
#include "MqttSettings.h"
#include "MessageOutput.h"
#include "version.h"

MqttSettingsClass::MqttSettingsClass()
{
}

void MqttSettingsClass::init()
{
    _mqttState = mqtt_idle;
    t_last_check_for_prerequisites = 0;
    t_wait_until_reconnect = 0;
    clientId = "esp32-CD6B3C";
    willTopic = "status";
    mqttClient = nullptr;
    _onMessageCallback = nullptr;

    createMqttClientObject();
}

void MqttSettingsClass::onMqttConnect(bool sessionPresent)
{
    _mqttState = mqtt_ready;
    String t0 = NtpSettings.getLocalTimeAndDate();
    MessageOutput.printf("Connected to MQTT at %s. _mqttState = mqtt_ready\r\n", t0.c_str());
    publish(willTopic, "online since " + t0);
    publish("version", ESP_Unterverteilung::version);
}

void MqttSettingsClass::onMqttDisconnect(espMqttClientTypes::DisconnectReason reason)
{
    t_wait_until_reconnect = millis();
//    delete mqttClient;
//    mqttClient = nullptr;
    _mqttState = mqtt_wait_for_reconnect;

    MessageOutput.println("Disconnected from MQTT. _mqttState = mqtt_wait_for_reconnect");

    MessageOutput.print("Disconnect reason: ");
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
}

void MqttSettingsClass::onMqttMessage(const espMqttClientTypes::MessageProperties& properties, const char* topic, const uint8_t* payload, size_t len, size_t index, size_t total)
{
    char *strval = new char[len + 1];
    memcpy(strval, payload, len);
    strval[len] = 0;
    String subtopic(topic + getPrefix().length());

    _onMessageCallback(subtopic, String(strval));

    delete[] strval;
}

void MqttSettingsClass::performConnect()
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    using std::placeholders::_3;
    using std::placeholders::_4;
    using std::placeholders::_5;
    using std::placeholders::_6;

    MessageOutput.println("Connecting to MQTT...");
    static_cast<espMqttClient *>(mqttClient)->setServer("nas-2", 1883);
    static_cast<espMqttClient *>(mqttClient)->setCredentials("", "");
    static_cast<espMqttClient *>(mqttClient)->setWill(willTopic.c_str(), 2, true, "offline");
    static_cast<espMqttClient *>(mqttClient)->setClientId(clientId.c_str());
    static_cast<espMqttClient *>(mqttClient)->onConnect(std::bind(&MqttSettingsClass::onMqttConnect, this, _1));
    static_cast<espMqttClient *>(mqttClient)->onDisconnect(std::bind(&MqttSettingsClass::onMqttDisconnect, this, _1));
    static_cast<espMqttClient *>(mqttClient)->onMessage(std::bind(&MqttSettingsClass::onMqttMessage, this, _1, _2, _3, _4, _5, _6));
    mqttClient->connect();
}

bool MqttSettingsClass::isConnected()
{
    return _mqttState == mqtt_ready;
}

String MqttSettingsClass::getPrefix()
{
    return "unterverteilung/";
}

void MqttSettingsClass::publish(const String& subtopic, const String& payload)
{
    assert(_mqttState == mqtt_ready);

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

void MqttSettingsClass::subscribe (const String &subtopic)
{
    assert(_mqttState == mqtt_ready);

    String topic = getPrefix();
    topic += subtopic;

    MessageOutput.printf("Subscription for topic: %s\r\n", topic.c_str());
    mqttClient->subscribe(topic.c_str(), 0);
}

void MqttSettingsClass::loop()
{
    unsigned long t_current = millis();

    if((_mqttState == mqtt_idle) && (t_current - t_last_check_for_prerequisites > 1000))
    {
        t_last_check_for_prerequisites = t_current;
        if (NetworkSettings.isConnected() && NtpSettings.is_ready())
        {
            _mqttState = mqtt_initializing;
            MessageOutput.println("_mqttState = mqtt_initializing");
            performConnect();
        }
    }

    if (_mqttState == mqtt_wait_for_reconnect && (t_current - t_wait_until_reconnect) > 5000)
    {
        _mqttState = mqtt_idle;
        MessageOutput.println("_mqttState = mqtt_idle (after mqtt_wait_for_reconnect)");
//        createMqttClientObject();
    }
}

void MqttSettingsClass::createMqttClientObject()
{
    assert(_mqttState == mqtt_idle);
    assert(mqttClient == nullptr);

    mqttClient = static_cast<MqttClient*>(new espMqttClient);
}

void MqttSettingsClass::onMessageCallback (OnMessageCallback callback)
{
    _onMessageCallback = callback;
}

MqttSettingsClass MqttSettings;
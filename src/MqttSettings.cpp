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
    MessageOutput.logf("Connected to MQTT at %s. _mqttState = mqtt_ready", t0.c_str());
    publish(willTopic, MessageOutput.get_millis_as_String("millis=%d: ") + "mqtt connect at " + t0);
    publish("version", ESP_Unterverteilung::version);
}

void MqttSettingsClass::onMqttDisconnect(espMqttClientTypes::DisconnectReason reason)
{
    t_wait_until_reconnect = millis();
    _mqttState = mqtt_wait_for_reconnect;

    MessageOutput.logf("Disconnected from MQTT. _mqttState = mqtt_wait_for_reconnect");

    const char *fmt = "Disconnect reason: %s";
    switch (reason) {
    case espMqttClientTypes::DisconnectReason::TCP_DISCONNECTED:
        MessageOutput.logf(fmt, "TCP_DISCONNECTED");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_UNACCEPTABLE_PROTOCOL_VERSION:
        MessageOutput.logf(fmt, "MQTT_UNACCEPTABLE_PROTOCOL_VERSION");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_IDENTIFIER_REJECTED:
        MessageOutput.logf(fmt, "MQTT_IDENTIFIER_REJECTED");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_SERVER_UNAVAILABLE:
        MessageOutput.logf(fmt, "MQTT_SERVER_UNAVAILABLE");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_MALFORMED_CREDENTIALS:
        MessageOutput.logf(fmt, "MQTT_MALFORMED_CREDENTIALS");
        break;
    case espMqttClientTypes::DisconnectReason::MQTT_NOT_AUTHORIZED:
        MessageOutput.logf(fmt, "MQTT_NOT_AUTHORIZED");
        break;
    default:
        MessageOutput.logf(fmt, "Unknown");
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

    MessageOutput.logf("Connecting to MQTT...");
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

    MessageOutput.logf("Subscription for topic: %s", topic.c_str());
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
            MessageOutput.logf("_mqttState = mqtt_initializing");
            performConnect();
        }
    }

    if (_mqttState == mqtt_wait_for_reconnect && (t_current - t_wait_until_reconnect) > 5000)
    {
        _mqttState = mqtt_idle;
        MessageOutput.logf("_mqttState = mqtt_idle (after mqtt_wait_for_reconnect)");
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
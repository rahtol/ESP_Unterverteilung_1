#include "MqttS0Counters.h"
#include "MessageOutput.h"
#include "NtpSettings.h"


MqttS0CounterClass::MqttS0CounterClass(String name, uint8_t pin, uint32_t pulsesPerKwh) : _name(name)
{
     _pin = pin;
    _pulsesPerKwh = pulsesPerKwh;
    kwH_per_pulse = 1.0 / pulsesPerKwh;

    state = 0;
    count = 0.0;
    count_initial = 0.0;
    count_total = count_initial;
    t_state0_left = 0;
    t_last_mqtt_publish = 0;
    initial_count_subscription_callback_has_occured = false;
    this->init();
};

void MqttS0CounterClass::init() 
{
    MessageOutput.logf("%s: Setting pin=%d as INPUT", _name.c_str(), _pin);
    pinMode(_pin, INPUT);
};

const String &MqttS0CounterClass::getName()
{
    return _name;
}



void MqttS0CounterClass::loop()
{
    int pinstate = digitalRead(_pin);
    unsigned long t_current = millis();

    switch (pinstate) {
        case 0:
        switch (state) {
            case 0:
            break;
            
            case 1:
            // impulse too short debouncing in action
            state = 0;
            MessageOutput.logf("1->0 %s", _name.c_str());
            break;

            case 2:
            // this is the regular transition back to idle
            state = 0;
            MessageOutput.logf("2->0 %s", _name.c_str());
            break;

            case 3:
            // finally signal recovered after error
            MessageOutput.logf("3->0 %s", _name.c_str());
            state = 0;
            break;

            default:
            // fatal, defensive action required
            MessageOutput.logf("Fatal(%s): Illegal state detected #1: %d", _name.c_str(), state);
            break;
        };
        break;

        case 1:
        switch (state) {
            case 0:
            state = 1;
            t_state0_left = t_current;
            MessageOutput.logf("0->1 %s", _name.c_str());
            break;
            
            case 1:
            break;

            case 2:
            break;

            case 3:
            break;

            default:
            // fatal, defensive action required
            MessageOutput.logf("Fatal(%s): Illegal state detected #2: %d", _name.c_str(), state);
            break;
        };
        break;

        default:
        // fatal, defensive action required
        MessageOutput.logf("Fatal(%s): Illegal pinstate detected by digitalRead #1: %d", _name.c_str(), pinstate);
        break;
    };

    // evaluate state dependent timeout
    switch (state) {
        case 0:
        break;

        case 1:
        if (t_current - t_state0_left > 10)
        {
            state = 2;
            countEvent();
        }
        break;

        case 2:
        if (t_current - t_state0_left > 1000)
        {
            state = 3;
            errEvent();
        }
        break;

        case 3:
        break;

        default:
        // fatal, defensive action required
        MessageOutput.logf("Fatal(%s): Illegal state detected #1: %d", _name.c_str(), state);
        break;
    };

    // check for publish event
    if (MqttSettings.isConnected() && (initial_count_subscription_callback_has_occured) && (t_current - t_last_mqtt_publish > 60000))
    {
        t_last_mqtt_publish = t_current;
        count_total += count;
        MqttSettings.publish(_name + "/count", String(count, 4));
        MqttSettings.publish(_name + "/count_total", String(count_total, 4));
        MqttSettings.publish(_name + "/state", String(state));
        MessageOutput.logf("Publish: name=%s state=%d, count=%.4f, count_total=%.4f", _name.c_str(), state, count, count_total);
        count = 0.0;
    };
};

void MqttS0CounterClass::countEvent()
{
    count = count + kwH_per_pulse;
}

void MqttS0CounterClass::errEvent()
{
    MessageOutput.logf ("Error(%s): Input too long high.", _name.c_str());
};

void MqttS0CounterClass::init_subscriptions()
{
    MqttSettings.subscribe(_name + "/initial_counter");
};



MqttS0CountersClass::MqttS0CountersClass() : _cbS0List ()
{
};

void MqttS0CountersClass::init()
{

};

void MqttS0CountersClass::init_subscriptions()
{
    using std::placeholders::_1;
    using std::placeholders::_2;
    MqttSettings.onMessageCallback(std::bind(&MqttS0CountersClass::onMqttMessage, this, _1, _2));

    for (auto &s0 : _cbS0List)
    {
        s0.init_subscriptions();
    }
};

void MqttS0CountersClass::loop()
{
    if (s0CounterState == s0_idle && MqttSettings.isConnected())
    {
        s0CounterState = s0_ready;
        init_subscriptions();
    }

    if (s0CounterState == s0_ready)
    {
        for (auto &s0 : _cbS0List)
        {
            s0.loop();
        }
    }
};

void MqttS0CountersClass::addS0Counter(String name, int pin, int pulsesPerKwh)
{
    MqttS0CounterClass *pS0Counter = new MqttS0CounterClass(name, pin, pulsesPerKwh);
    _cbS0List.push_back(*pS0Counter);    
};

void MqttS0CountersClass::onMqttMessage(String subtopic, String payload)
{
    String sKey = subtopic.substring(0, subtopic.indexOf('/'));

    for (auto &s0 : _cbS0List)
    {
        if (s0.getName() == sKey)
        {
            s0.onMqttMessage(subtopic, payload);
        }
    }
}

void MqttS0CounterClass::onMqttMessage(String subtopic, String payload)
{

    initial_count_subscription_callback_has_occured = true;
    count_initial = payload.toDouble();
    if (count_initial > count_total)
    {
        count_total = count_initial;
        MessageOutput.logf("Subscription callback for S0-instance==%s, inital_counter=%.4f", _name.c_str(), count_initial);
        MqttSettings.publish(_name + "/time_counter_initialized", NtpSettings.getLocalTimeAndDate());
    }
    else
    {
        MessageOutput.logf("Ignored subscription callback for S0-instance==%s, inital_counter=%.4f, count_total=%.4f", _name.c_str(), count_initial, count_total);
    }
}


MqttS0CountersClass MqttS0Counters;

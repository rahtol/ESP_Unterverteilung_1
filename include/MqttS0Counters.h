#include <vector>
#include "MqttSettings.h"

class MqttS0CounterClass {
public:
    MqttS0CounterClass(String name, uint8_t pin, uint32_t pulsesPerKwh);
    void init();
    void loop();
    void init_subscriptions();
    void onMqttMessage(String subtopic, String payload);
    const String &getName();

private:
    const String _name;
    uint8_t _pin;
    uint32_t _pulsesPerKwh;

    uint8_t state; // 0=idle, 1=debounce, 2=active, 3=error
    double count;
    double count_total;
    double count_initial;  // set via subscription at the iobroker
    unsigned long t_state0_left;
    double kwH_per_pulse;

    unsigned long t_last_mqtt_publish;
    boolean initial_count_subscription_callback_has_occured;

    void countEvent();
    void errEvent();
};

class MqttS0CountersClass {
public:
    MqttS0CountersClass();
    void init();
    void loop();
    void addS0Counter(String name, int pin, int pulsesPerKwh);
    void onMqttMessage(String subtopic, String payload);

private:
    typedef enum S0CounterState_t {
        s0_idle,
        s0_ready
    } S0CounterState;

    S0CounterState s0CounterState;
    std::vector<MqttS0CounterClass> _cbS0List;

    void init_subscriptions();
};

extern MqttS0CountersClass MqttS0Counters;

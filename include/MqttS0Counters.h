#include <vector>
#include "MqttSettings.h"

class MqttS0CounterClass {
public:
    MqttS0CounterClass(String name, uint8_t pin, uint32_t pulsesPerKwh);
    void init();
    void loop();

private:
    const String _name;
    uint8_t _pin;
    uint32_t _pulsesPerKwh;

    uint8_t state; // 0=idle, 1=debounce, 2=active, 3=error
    double count;
    double count_total;
    double count_initial;
    unsigned long t_state0_left;
    double kwH_per_pulse;

    unsigned long t_last_mqtt_publish;

    void countEvent();
    void errEvent();
};

class MqttS0CountersClass {
public:
    MqttS0CountersClass();
    void init();
    void loop();
    void addS0Counter(String name, int pin, int pulsesPerKwh);

private:
    std::vector<MqttS0CounterClass> _cbS0List;
};

extern MqttS0CountersClass MqttS0Counters;

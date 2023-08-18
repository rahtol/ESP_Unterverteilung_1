#include <Arduino.h>
#include "MessageOutput.h"
#include "NetworkSettings.h"
#include "NtpSettings.h"
#include "MqttSettings.h"
#include "MqttS0Counters.h"

void setup()
{
    Serial.begin(115200);
    MessageOutput.println("Starting Esp-Unterverteilung");

    // Initialize WiFi
    MessageOutput.println("Initialize Network... ");
    NetworkSettings.init();
    MessageOutput.println("... done Initialize Network");

    // Initialize NTP
    MessageOutput.println("Initialize NTP... ");
    NtpSettings.init();
    MessageOutput.println("done");

    // Initialize MqTT
    MessageOutput.println("Initialize MqTT... ");
    MqttSettings.init();
    MessageOutput.println("... done initialize MqTT");

    // Initialize MqttS0Counters
    MessageOutput.println("Initialize MqttS0Counters... ");
    MqttS0Counters.init();
    MqttS0Counters.addS0Counter("küche", 5, 2000);  // Küche, grün
    MqttS0Counters.addS0Counter("herd", 4, 400);  // Herd, gelb
    MessageOutput.println("... done initialize MqttS0Counters... ");
}

void loop()
{
  MessageOutput.loop();
  yield();
  NetworkSettings.loop();
  yield();
  MqttS0Counters.loop();
  yield();
}

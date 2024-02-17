#include <Arduino.h>
#include "MessageOutput.h"
#include "NetworkSettings.h"
#include "NtpSettings.h"
#include "MqttSettings.h"
#include "MqttS0Counters.h"
#include "version.h"

namespace ESP_Unterverteilung {

//const char* version = "Project ESP_Unterverteilung, Version 1.0, 20.08.2023";
//const char* version = "Project ESP_Unterverteilung, Version 1.1, 21.08.2023";
//const char* version = "Project ESP_Unterverteilung, Version 1.2, 21.08.2023";
//const char* version = "Project ESP_Unterverteilung, Version 1.3, 25.08.2023";
//const char* version = "Project ESP_Unterverteilung, Version 1.4, 14.02.2024";
//const char* version = "Project ESP_Unterverteilung, Version 1.5, 15.02.2024";
//const char* version = "Project ESP_Unterverteilung, Version 1.6, 16.02.2024";
const char* version = "Project ESP_Unterverteilung, Version 1.7, 17.02.2024";

}


void setup()
{
    Serial.begin(115200);
    MessageOutput.println(ESP_Unterverteilung::version);

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
    MqttS0Counters.addS0Counter("küche", 5, 1000);  // Küche, grün
    MqttS0Counters.addS0Counter("herd", 4, 400);  // Herd, gelb
    MessageOutput.println("... done initialize MqttS0Counters... ");
}

void loop()
{
  MessageOutput.loop();
  yield();
  NetworkSettings.loop();
  yield();
  NtpSettings.loop();
  yield();
  MqttSettings.loop();
  yield();
  MqttS0Counters.loop();
  yield();
}

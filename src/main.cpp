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
//const char* version = "Project ESP_Unterverteilung, Version 1.7, 17.02.2024";
//const char* version = "Project ESP_Unterverteilung, Version 1.12, 17.02.2024 17:51:00";
const char* version = "Project ESP_Unterverteilung, Version 1.13, 20.02.2024 16:47:00";

}


void setup()
{
    Serial.begin(115200);
    MessageOutput.logf(ESP_Unterverteilung::version);

    // Initialize WiFi
    MessageOutput.logf("Initialize Network... ");
    NetworkSettings.init();
    MessageOutput.logf("... done Initialize Network");

    // Initialize NTP
    MessageOutput.logf("Initialize NTP... ");
    NtpSettings.init();
    MessageOutput.logf("done");

    // Initialize MqTT
    MessageOutput.logf("Initialize MqTT... ");
    MqttSettings.init();
    MessageOutput.logf("... done initialize MqTT");

    // Initialize MqttS0Counters
    MessageOutput.logf("Initialize MqttS0Counters... ");
    MqttS0Counters.init();
    MqttS0Counters.addS0Counter("küche", 5, 1000);  // Küche, grün
    MqttS0Counters.addS0Counter("herd", 4, 400);  // Herd, gelb
    MessageOutput.logf("... done initialize MqttS0Counters... ");
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

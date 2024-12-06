#include <Arduino.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
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
//const char* version = "Project ESP_Unterverteilung, Version 1.13, 20.02.2024 16:47:00";
//const char* version = "Project ESP_Unterverteilung, Version 1.14, 22.02.2024 11:12:00";
//const char* version = "Project ESP_Unterverteilung, Version 1.16, 05.12.2024 19:02:00";
//const char* version = "Project ESP_Unterverteilung, Version 1.17, 05.12.2024 20:52:00";
//const char* version = "Project ESP_Unterverteilung, Version 1.18, 06.12.2024 13:42:00";
const char* version = "Project ESP_Unterverteilung, Version 1.19, 06.12.2024 17:04";

}

// Create an AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncWebSocket ws("/console");
String boot_time_amd_date = "??";

extern const char index_html[] asm("_binary_src_index_html_start");
extern const uint8_t index_html_end[] asm("_binary_src_index_html_end");

String to_String(double f)
{
  char bf [32];
  sprintf(bf, "%3.2f", f);
  return String(bf);
}

// Replaces placeholder with button section in your web page
String processor(const String& var){
  //Serial.println(var);
  if(var == "COUNTERPLACEHOLDER"){
    String countervalues = "";
    countervalues +=  "<h4>Kueche: " + to_String(MqttS0Counters.get_count("küche")) + "</h4>";
    countervalues += "<h4>Herd: " + to_String(MqttS0Counters.get_count("herd")) + "</h4>";
    return countervalues;
  }
  else if (var == "BOOTTIMEANDDATE") {
    return "<h4>Up since: " + boot_time_amd_date + "</h4>";
  }
  else if (var == "VERSION") {
    return "<h4>" + String(ESP_Unterverteilung::version) + "</h4>";
  }
  else if (var == "NOW") {
    return "<h4> counter values at " + NtpSettings.getLocalTimeAndDate() + "</h4>";
  }
  return String();
}


void onWebsocketEvent(AsyncWebSocket * server, AsyncWebSocketClient * client, AwsEventType type, void * arg, uint8_t *data, size_t len)
{
  if(type == WS_EVT_CONNECT)
  {
    //client connected
    Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
    client->printf("%s\n", ESP_Unterverteilung::version);
    client->ping();
  }
  else if (type == WS_EVT_DISCONNECT)
  {
    //client disconnected
    Serial.printf("ws[%s][%u] disconnect: %u\n", server->url(), client->id(), 0);
  }  
  else if(type == WS_EVT_ERROR)
  {
    //error was received from the other end
    Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
  }
  else if(type == WS_EVT_PONG)
  {
    //pong message was received (in response to a ping request maybe)
    Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len)?(char*)data:"");
  } 
  else if(type == WS_EVT_DATA)
  {
    //data packet
    AwsFrameInfo * info = (AwsFrameInfo*)arg;
    Serial.printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT)?"text":"binary", info->len);
  }
}


void setup()
{
    using namespace std::placeholders;

    Serial.begin(115200);
    MessageOutput.logf(ESP_Unterverteilung::version);

    // Initialize WiFi
    MessageOutput.logf("Initialize Network... ");
    NetworkSettings.init();
    MessageOutput.logf("... done Initialize Network");

    ArduinoOTA
      .onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
          type = "sketch";
        } else {  // U_SPIFFS
          type = "filesystem";
        }

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
      })
      .onEnd([]() {
        Serial.println("\nEnd");
      })
      .onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
      })
      .onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
          Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
          Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
          Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
          Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
          Serial.println("End Failed");
        }
      });

    ArduinoOTA.begin();


    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(200, "text/html", index_html, processor);
    });

    // Enable Websocket handling
    server.addHandler(&ws);
    ws.onEvent(std::bind(&onWebsocketEvent, _1, _2, _3, _4, _5, _6));
    MessageOutput.setWebsocket(&ws);

    // Start server
    server.begin();

    // Initialize NTP
    MessageOutput.logf("Initialize NTP... ");
    NtpSettings.init();
    MessageOutput.logf("done");

    boot_time_amd_date = NtpSettings.getLocalTimeAndDate();

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
  ArduinoOTA.handle();
  ws.cleanupClients();
  
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

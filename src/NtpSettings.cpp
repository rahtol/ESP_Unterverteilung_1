#include <Arduino.h>
#include "NtpSettings.h"
#include <time.h>

NtpSettingsClass::NtpSettingsClass()
{
}

void NtpSettingsClass::init()
{
    setServer();
    setTimezone();
}

void NtpSettingsClass::setServer()
{
    configTime(0, 0, "pool.ntp.org");
}

void NtpSettingsClass::setTimezone()
{
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
}

String NtpSettingsClass::getLocalTimeAndDate()
{
  struct tm timeinfo;
  if(!::getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return String("???");
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  
  char str[32];
  strftime(str, sizeof str, "%d.%m.%Y:%H.%M.%S", &timeinfo);
  return String(str);
};


NtpSettingsClass NtpSettings;

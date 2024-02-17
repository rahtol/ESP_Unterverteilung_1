#include <Arduino.h>
#include "NtpSettings.h"
#include <time.h>

NtpSettingsClass::NtpSettingsClass()
{
}

void NtpSettingsClass::init()
{
    _ntpState = ntp_idle;
    t_last_call_to_getlocaltime = 0;

    using std::placeholders::_1;
    NetworkSettings.onEvent(std::bind(&NtpSettingsClass::NetworkEvent, this, _1));

    setServer();
    setTimezone();
    t_boot = millis();
};

void NtpSettingsClass::loop ()
{
    unsigned long t_current = millis();
    if ((_ntpState == ntp_initializing) && (t_current - t_last_call_to_getlocaltime > 1000))
    {
        t_last_call_to_getlocaltime = t_current;
        struct tm timeinfo;
        if(::getLocalTime(&timeinfo))
        {
            _ntpState = ntp_ready;
        }
    }
};

void NtpSettingsClass::setServer()
{
    configTime(0, 0, "pool.ntp.org");
}

void NtpSettingsClass::setTimezone()
{
    setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);
    tzset();
}

bool NtpSettingsClass::is_ready()
{
    return _ntpState == ntp_ready;
};


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

void NtpSettingsClass::NetworkEvent(network_event event)
{
    switch (event) {
    case network_event::NETWORK_GOT_IP:
        _ntpState = ntp_initializing;
        break;
    case network_event::NETWORK_DISCONNECTED:
        _ntpState = ntp_idle;
        break;
    default:
        break;
    }
}

unsigned long NtpSettingsClass::get_boottime_millis()
{
    return t_boot;
};


NtpSettingsClass NtpSettings;

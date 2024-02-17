#pragma once

#include <WString.h>
#include "NetworkSettings.h"

class NtpSettingsClass {
public:
    NtpSettingsClass();
    void init();
    void loop();

    bool is_ready();
    String getLocalTimeAndDate();
    unsigned long get_boottime_millis();

private:
    typedef enum NtpState_t {
        ntp_idle,
        ntp_initializing,
        ntp_ready
    } NtpState;
    NtpState _ntpState;
    unsigned long t_last_call_to_getlocaltime;
    unsigned long t_boot;

    void setServer();
    void setTimezone();
    void NetworkEvent(network_event event);
};

extern NtpSettingsClass NtpSettings;

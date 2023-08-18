#pragma once

#include <WString.h>

class NtpSettingsClass {
public:
    NtpSettingsClass();
    void init();

    void setServer();
    void setTimezone();
    String getLocalTimeAndDate();

};

extern NtpSettingsClass NtpSettings;
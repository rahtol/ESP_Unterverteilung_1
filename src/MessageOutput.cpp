// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */
#include "MessageOutput.h"

#include <Arduino.h>

MessageOutputClass MessageOutput;

String MessageOutputClass::get_millis_as_String (const char *fmt)
{
  char str[32];
  unsigned long t_current = millis();
  sprintf(str, (fmt==NULL ? "%08d" : fmt), &t_current);
  return String(str);
}

size_t MessageOutputClass::write(uint8_t c)
{
    return Serial.write(c);
}

size_t MessageOutputClass::write(const uint8_t* buffer, size_t size)
{
    return Serial.write(buffer, size);
}

void MessageOutputClass::logf(const char *fmt...)
{
    va_list args;
    va_start(args, fmt);

    char str[32];
    unsigned long t_current = millis();
    sprintf(str, "%08d: ", &t_current);
    Serial.write((uint8_t *)str, strlen(str));

    printf(fmt, args);

    Serial.write('\r');
    Serial.write('\n');
}


void MessageOutputClass::loop()
{
}
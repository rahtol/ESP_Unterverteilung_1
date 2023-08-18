// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Copyright (C) 2022 Thomas Basler and others
 */
#include "MessageOutput.h"

#include <Arduino.h>

MessageOutputClass MessageOutput;

size_t MessageOutputClass::write(uint8_t c)
{
    return Serial.write(c);
}

size_t MessageOutputClass::write(const uint8_t* buffer, size_t size)
{
    return Serial.write(buffer, size);
}

void MessageOutputClass::loop()
{
}
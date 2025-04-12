#pragma once

#include <Arduino.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#include <string>

class LCD {
public:
    LCD();
    static void init();
    static void displayValues(const int a, const int b, const int c, const int d);
    static void print(const String& text, const uint8_t row = 0,  const bool clear = false);
private:
    static LiquidCrystal_I2C lcd;
};
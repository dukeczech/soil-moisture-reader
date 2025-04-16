#include "lcd.h"

LiquidCrystal_I2C LCD::lcd(0x27, 16, 2);

LCD::LCD() {}

void LCD::init() {
    lcd.backlight();
    lcd.init();
}

void LCD::backlight(const bool on) {
    if (on)
        lcd.backlight();
    else
        lcd.noBacklight();
}

void LCD::displayValues(const int a, const int b, const int c, const int d) {
    lcd.clear();
    lcd.printf("A: %3d%%  B: %3d%%", a, b);
    lcd.setCursor(0, 1);
    lcd.printf("C: %3d%%  D: %3d%%", c, d);
}

void LCD::print(const String& text, const uint8_t row, const bool clear) {
    if (clear) lcd.clear();
    lcd.setCursor(0, row);
    lcd.print(text);
}
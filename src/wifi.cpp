
#include <ESP8266WiFi.h>

#include "wifi.h"
#include "lcd.h"

String WifiManager::m_progress = "";

WifiManager::WifiManager() {}

bool WifiManager::connect(const char* ssid, const char* passphrase) {
    Serial.println();
    Serial.print("Connecting to wifi...");

    // Maximum tx power
    WiFi.setOutputPower(20.5);

    // Use only B-band
    WiFi.setPhyMode(WIFI_PHY_MODE_11B);

    LCD::print("Wifi connecting", 0, true);

    WiFi.begin(ssid, passphrase);
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        // Show the progress on LCD
        incrementProgress();

        delay(500);
        // ESP.restart();
    }

    const String ip = WiFi.localIP().toString();

    Serial.println("");
    Serial.print("Connected to wifi [");
    Serial.print(ip);
    Serial.println("]");

    String pad = String("                ");
    pad.remove((16 - ip.length()) / 2);
    LCD::print("      IP:", 0, true);
    LCD::print(pad + ip, 1, false);

    return true;
}

void WifiManager::incrementProgress() {
    if (m_progress.isEmpty())
        m_progress = ".";
    else
        m_progress += ".";

    if (m_progress.length() > 16) m_progress = ".";

    // Center the progress
    String pad = "                ";
    pad.remove((16 - m_progress.length()) / 2);
    if (m_progress.length() % 2 != 0) pad += " ";
    LCD::print(pad + m_progress + pad, 1, false);
}
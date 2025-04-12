#include "ota.h"

#include <ArduinoOTA.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>

#include "lcd.h"

void OTA::setup(const char* ssid, const char* password) {
    // Skip if already connected
    if (WiFi.isConnected()) {
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
    }

    while (WiFi.waitForConnectResult() != WL_CONNECTED) {
        Serial.println("Connection failed! Rebooting...");
        delay(5000);
        ESP.restart();
    }

    // Port defaults to 8266
    // ArduinoOTA.setPort(8266);

    // Hostname defaults to esp8266-[ChipID]
    // ArduinoOTA.setHostname("myesp8266");

    // No authentication by default
    // ArduinoOTA.setPassword("admin");

    // Password can be set with it's md5 value as well
    // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
    // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {  // U_FS
            type = "filesystem";
        }

        // NOTE: if updating FS this would be the place to unmount FS using FS.end()
        Serial.println("Start updating " + type);
        LCD::print("  FW update...", 0, true);
    });
    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
        LCD::print("  FW update OK", 0, true);
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        const int num = (progress / (total / 100));
        Serial.printf("Progress: %u%%\n", num);

        if(num % 2 == 0) {
            const String status = String("      ") + String(progress / (total / 100), 10) + String("%");
            LCD::print(status, 1, false);
        }
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End failed");
        }
    });
    ArduinoOTA.begin();

    Serial.print("Ready, IP address: ");
    Serial.println(WiFi.localIP());
}

void OTA::loop() {
    ArduinoOTA.handle();
}

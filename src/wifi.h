#pragma once

#include <Arduino.h>

class WifiManager {
   public:
    WifiManager();

    static bool connect(const char* ssid, const char* passphrase);

   private:
    static String m_progress;

    static void incrementProgress();
};
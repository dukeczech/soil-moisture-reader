#pragma once

#include <Arduino.h>

#include <NTPClient.h>
//#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

class Time
{
private:
    static WiFiUDP m_ntpUDP;
    static NTPClient m_timeClient;
public:
    Time();
    ~Time();

    static void init();
    static void loop();

    static int getHours();

    static String getTime();
    static String getDate();
};



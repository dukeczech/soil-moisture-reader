#include "ntptime.h"

WiFiUDP Time::m_ntpUDP;
NTPClient Time::m_timeClient(m_ntpUDP, 3600 + 3600);

Time::Time() {}

Time::~Time() {}

void Time::init() {
    m_timeClient.begin();
}

void Time::loop() {
    m_timeClient.update();
}

String Time::getTime() {
    return m_timeClient.getFormattedTime();
}

String Time::getDate() {
    return "TODO";
}
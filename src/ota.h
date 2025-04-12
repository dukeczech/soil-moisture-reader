#ifndef STASSID
#define STASSID "your-ssid"
#define STAPSK "your-password"
#endif

class OTA {
   public:
    static void setup(const char* ssid, const char* password);

    static void loop();
};
#include <Arduino.h>
#include <WiFi.h>
#include "secrets.h"

void setup() {
    Serial.begin(115200);
    delay(2000);

    Serial.println();
    Serial.println("=== ESP32-C3 WIFI TEST ===");

    WiFi.mode(WIFI_AP);
    delay(500);

    bool ok = WiFi.softAP(
        AP_SSID,
        AP_PASS,
        1,      // channel 1
        false,  // không ẩn SSID
        4
    );

    Serial.print("softAP = ");
    Serial.println(ok ? "OK" : "FAIL");

    Serial.print("SSID = ");
    Serial.println(AP_SSID);

    Serial.print("IP = ");
    Serial.println(WiFi.softAPIP());

    Serial.print("MAC = ");
    Serial.println(WiFi.softAPmacAddress());

    Serial.print("Channel = ");
    Serial.println(WiFi.channel());
}

void loop() {
    Serial.print("TEST uptime_ms=");
    Serial.print(millis());

    Serial.print(" AP=");
    Serial.print(WiFi.getMode() == WIFI_AP ? "OK" : "NO");

    Serial.print(" clients=");
    Serial.println(WiFi.softAPgetStationNum());

    delay(5000);
}
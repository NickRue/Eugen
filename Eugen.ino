#include <BlynkSimpleEsp8266.h>           // Library to use the Blynk-App

#include <WiFiManager.h>                  // Library to enable wifi controll
WiFiManager wifiManager;
WiFiClient client;

char blynkAuth[] = "fW8dET98PxDO2kyxpZqzY03pjkUzjEuc";


void setup() {
  Serial.begin(9600);

    // Also accessible at IP 192.168.4.1
  wifiManager.autoConnect("Eugen");      // Setup the name of the hotspot

  Blynk.begin(blynkAuth, WiFi.SSID().c_str(), WiFi.psk().c_str(), "iot.informatik.uni-oldenburg.de", 8080);        // Connect to Blynk-Client

}
void loop() {
  
}
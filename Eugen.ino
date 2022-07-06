#include <WiFiManager.h>                  // Library to enable wifi controll
WiFiManager wifiManager;
WiFiClient client;




void setup() {
  Serial.begin(9600);

    // Also accessible at IP 192.168.4.1
  wifiManager.autoConnect("Eugen");      // Setup the name of the hotspot

}
void loop() {
  
}
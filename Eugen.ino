#include <BlynkSimpleEsp8266.h> // Library to use the Blynk-App

#include <WiFiManager.h> // Library to enable wifi controll
WiFiManager wifiManager;
WiFiClient client;

#include <SPI.h> // Library to controll the LED-Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Servo.h"
Servo myservo;
int angle  = 0;
#define SERVO_PIN D7 // Data pin for the servo

#define BUTTON_PIN D3 // Pin on which to read the button input

#define BUZZER_PIN D5 // Pinn on which the buzzer reads
#define c  1915 //Note c

#define OLED_RESET 0 // "0" for ESP8266
Adafruit_SSD1306 display(OLED_RESET);

char blynkAuth[] = "fW8dET98PxDO2kyxpZqzY03pjkUzjEuc";

boolean koffeePouring = false;

void setup()
{
  Serial.begin(9600);
  myservo.attach(SERVO_PIN);
  digitalWrite(SERVO_PIN, LOW);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize the OLED-Display
  display.display();

  display.clearDisplay(); // Display connection status
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("Verbinde");
  display.println("dich mit");
  display.println("der SSID ");
  display.println("Eugen");
  display.setCursor(0, 0);
  display.display();

  // Also accessible at IP 192.168.4.1
  wifiManager.autoConnect("Eugen"); // Setup the name of the hotspot

  display.clearDisplay(); // Display connection status
  display.println("Verbunden");
  display.setCursor(0, 0);
  display.display();

  Blynk.begin(blynkAuth, WiFi.SSID().c_str(), WiFi.psk().c_str(), "iot.informatik.uni-oldenburg.de", 8080); // Connect to Blynk-Client
}


void loop()
{
  Blynk.run(); // Run the Blynk-Client
  
  int buttonState = digitalRead(BUTTON_PIN); // Read the button
  if(buttonState == LOW) // Low indicates button push
  {
    if(!koffeePouring)
      pourCoffee();
  }
}

void pourCoffee()
{
  koffeePouring = true;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Kaffee");
  display.display();

  myservo.write(45);
  delay(500);
  myservo.write(135);
  delay(500);

  digitalWrite(SERVO_PIN, LOW);

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Warte auf Kaffee");
  display.display();
  koffeePouring = false;

  playTone()
}

BLYNK_WRITE(V0)
{
  int i = param.asInt();
  if (i == 0 || koffeePouring)
    return;

  pourCoffee(c, 500); // Play a tone to indicate that the coffee is poured
}

//Funktion to play a frequency
//Expects a frequency (tone) and a duration (duration)
void playTone(int tone, int duration) {
  for (long i = 0; i < duration * 1000L; i += tone * 2) {
    digitalWrite(BUZZER_PIN, HIGH);
    delayMicroseconds(tone);
    digitalWrite(BUZZER_PIN, LOW);
    delayMicroseconds(tone);
  }
}
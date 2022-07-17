#include <BlynkSimpleEsp8266.h> // Library to use the Blynk-App
#define BLYNK_PRINT Serial

#include <WiFiManager.h> // Library to enable wifi controll
WiFiManager wifiManager;
WiFiManagerParameter blynkToken("blynkToken", "Blynk Token", "", 33);
WiFiClient client;

#include <SPI.h> // Library to controll the LED-Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Servo.h" // Library to controll the servo
Servo myservo;

#include "pitches.h" // Library contains the pitches of the notes

#include <Hash.h> // Library to use the hash function
#include <FS.h>   // Library to use the filesystem

#define SERVO_PIN D7 // Data pin for the servo

#define BUTTON_PIN D3              // Pin on which to read the button input
const int SHORT_PRESS_TIME = 3000; // 3 seconds
int lastState = HIGH;              // the previous state from the input pin
int currentState;                  // the current reading from the input pin
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;

#define BUZZER_PIN D5 // Pinn on which the buzzer reads

#define OLED_RESET 0 // "0" for ESP8266
Adafruit_SSD1306 display(OLED_RESET);

boolean koffeePouring = false;

String readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Reading file: %s\r\n", path);
  File file = fs.open(path, "r");
  if (!file || file.isDirectory())
  {
    Serial.println("- empty file or failed to open file");
    return String();
  }
  Serial.println("- read from file:");
  String fileContent;
  while (file.available())
  {
    fileContent += String((char)file.read());
  }
  file.close();
  Serial.println(fileContent);
  return fileContent;
}

void writeFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Writing file: %s\r\n", path);
  File file = fs.open(path, "w");
  if (!file)
  {
    Serial.println("- failed to open file for writing");
    return;
  }
  if (file.print(message))
  {
    Serial.println("- file written");
  }
  else
  {
    Serial.println("- write failed");
  }
  file.close();
}

void setup()
{
  Serial.begin(9600);

  myservo.attach(SERVO_PIN);
  digitalWrite(SERVO_PIN, LOW);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize the OLED-Display
  display.display();
  delay(2000);        // display startup animation
  if (SPIFFS.begin()) // Mount the file system
  {
    Serial.println("SPIFFS mounted"); // Debug message
  }
  else
  {
    Serial.println("SPIFFS mount failed");
    display.clearDisplay(); // Display the error message
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.println("SPIFFS mount failed");
    display.display();
    return; // Stop the program, if the file system could not be mounted
  }

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
  String tokenString = readFile(SPIFFS, "/blynkToken.txt");
  if (!tokenString.isEmpty())
  {
    blynkToken.setValue(tokenString.c_str(), 33);
  }
  wifiManager.addParameter(&blynkToken);

  wifiManager.autoConnect("Eugen");                            // Setup the name of the hotspot
  writeFile(SPIFFS, "/blynkToken.txt", blynkToken.getValue()); // Write the blynk token to the filesystem

  Blynk.connectWiFi(WiFi.SSID().c_str(), WiFi.psk().c_str()); // Connect to the wifi network
  Blynk.config(blynkToken.getValue(), "iot.informatik.uni-oldenburg.de", 8080);
  bool connection = Blynk.connect(10000); // Connect to the Blynk server with a timeout of 10 seconds

  if (!connection)
  {
    display.clearDisplay(); // Display the error message
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.println("Verbindungsfehler");
    display.display();
    delay(2000);
    if (Blynk.isTokenInvalid()) // Check if the token is valid
    {
      Serial.println("Invalid token, please check your Blynk App");
      display.clearDisplay(); // Display connection status
      display.setCursor(0, 0);
      display.println("Token");
      display.println("ungueltig");
      display.println("Geraet wird");
      display.println("neugestartet");
      display.display();
      delay(5000);
    }
    wifiManager.erase(); //Erase credentials from the filesystem if the connection failed
    ESP.restart(); // Restart the program, if the connection could not be established
  }

  display.clearDisplay(); // Display connection status
  display.println("Verbunden");
  display.setCursor(0, 0);
  display.display();
  delay(2000);

  display.clearDisplay(); // Display status
  display.setCursor(0, 0);
  display.println("Bereit");
  display.println("einen");
  display.println("Kaffee");
  display.println("zu machen");
  display.display();
}

void loop()
{
  // read the state of the switch/button:
  currentState = digitalRead(BUTTON_PIN);

  if (lastState == HIGH && currentState == LOW) // button is pressed
    pressedTime = millis();
  else if (lastState == LOW && currentState == HIGH)
  { // button is released
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration > SHORT_PRESS_TIME) // long press, erase wifimanager and restart
    {
      wifiManager.erase();
      display.setCursor(0, 0);
      display.clearDisplay(); // Display connection status
      display.println("Verbindung");
      display.println("geloescht");
      display.display();
      delay(5000);
      ESP.restart();
    }
    else
      pourCoffee(); // short press, pour coffee
  }
  lastState = currentState;

  Blynk.run(); // Run the Blynk-Client
}

void pourCoffee()
{
  koffeePouring = true;

  display.clearDisplay(); // Display that a coffee is beeing poured
  display.setCursor(0, 0);
  display.println("Mache");
  display.println("Kaffee");
  display.display();

  myservo.write(45); // Rotate the servo so that it presses the button on the coffee machine
  delay(1000);
  myservo.write(150);
  delay(1000);

  digitalWrite(SERVO_PIN, LOW);

  display.clearDisplay(); // Display that the controler is ready for the next coffee
  display.setCursor(0, 0);
  display.println("Bereit");
  display.println("einen Kaffee");
  display.println("zu machen");
  display.display();
  ;
  koffeePouring = false;

  tone(BUZZER_PIN, NOTE_C4, 500); // Play a tone to indicate that the coffee is poured
}

BLYNK_WRITE(V0) // Coffee button in blynk app is pressed
{
  int i = param.asInt();
  if (i == 0 || koffeePouring)
    return;

  pourCoffee();
}
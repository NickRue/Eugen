#include <BlynkSimpleEsp8266.h> // Library to use the Blynk-App

#include <WiFiManager.h>                                              // Library to enable wifi controll
WiFiManager wifiManager;                                              // Create a wifi manager object
WiFiManagerParameter blynkToken("blynkToken", "Blynk Token", "", 33); // Parameter to set the Blynk Token

#include <SPI.h> // Libraries to controll the LED-Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Servo.h" // Library to controll the servo
Servo myservo;     // Create a servo object

#include "pitches.h" // Library contains pitches for notes

#include <FS.h> // Library to use the filesystem

#define SERVO_PIN D7 // Data pin for the servo

#define BUTTON_PIN D3              // Pin on which to read the button input
const int SHORT_PRESS_TIME = 3000; // 3 seconds is short press
int lastState = HIGH;              // the previous state from the input pin
int currentState;                  // the current reading from the input pin
unsigned long pressedTime = 0;     // the time the button was pressed
unsigned long releasedTime = 0;    // the time the button was released

#define BUZZER_PIN D5 // Pin on which the buzzer reads

#define OLED_RESET 0 // "0" for ESP8266
Adafruit_SSD1306 display(OLED_RESET);

boolean koffeePouring = false;

String readFile(fs::FS &fs, const char *path) // read the content of a file
{
  Serial.printf("Reading file: %s\r\n", path); // Debug message
  File file = fs.open(path, "r");              // Open the file
  if (!file || file.isDirectory())             // If the file doesn't exist or is a directory
  {
    Serial.println("- empty file or failed to open file"); // Debug message
    return String();                                       // Return an empty string
  }
  Serial.println("- read from file:"); // Debug message
  String fileContent;                  // Create a string to hold the content of the file
  while (file.available())             // While there is data to read
  {
    fileContent += String((char)file.read()); // Add the character to the string
  }
  file.close();                // Close the file
  Serial.println(fileContent); // Debug message
  return fileContent;          // Return the content of the file
}

void writeFile(fs::FS &fs, const char *path, const char *message) // write to a file
{
  Serial.printf("Writing file: %s\r\n", path); // Debug message
  File file = fs.open(path, "w");              // Open the file
  if (!file)                                   // If the file doesn't exist
  {
    Serial.println("- failed to open file for writing"); // Debug message
    return;
  }
  if (file.print(message)) // Write the message to the file
  {
    Serial.println("- file written"); // Debug message
  }
  else
  {
    Serial.println("- write failed"); // Debug message
  }
  file.close(); // Close the file
}

void setup()
{
  Serial.begin(9600); // Start the serial communication

  myservo.attach(SERVO_PIN);         // Attach the servo to the pin
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set the button pin to input with pullup
  pinMode(BUZZER_PIN, OUTPUT);       // Set the buzzer pin to output

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize the OLED-Display
  display.display();                         // display startup animation
  delay(2000);
  if (SPIFFS.begin()) // Mount the file system
  {
    Serial.println("SPIFFS mounted"); // Debug message
  }
  else
  {
    Serial.println("SPIFFS mount failed"); // Debug message
    display.clearDisplay();                // Display the error message
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("SPIFFS mount failed"); // Display the error message
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

  String tokenString = readFile(SPIFFS, "/blynkToken.txt"); // Read the token from the file
  if (!tokenString.isEmpty())
  {
    blynkToken.setValue(tokenString.c_str(), 33); // If a token was found, set the token to the html-page input
  }
  wifiManager.addParameter(&blynkToken); // Add the token as parameter to the wifi manager

  // Also accessible at IP 192.168.4.1
  wifiManager.autoConnect("Eugen");                            // Setup the name of the hotspot and connect to wifi
  writeFile(SPIFFS, "/blynkToken.txt", blynkToken.getValue()); // Write the blynk token to the filesystem

  Blynk.config(blynkToken.getValue(), "iot.informatik.uni-oldenburg.de", 8080);
  display.clearDisplay(); // Display the connection status
  display.setCursor(0, 0);
  display.println("Verbindung");
  display.println("zu Blynk");
  display.println("wird");
  display.println("herge-");
  display.println("stellt");
  display.display();
  bool connection = Blynk.connect(15000); // Connect to the Blynk server with a timeout of 15 seconds

  if (!connection) // If the no connection to the Blynk app could be established
  {
    display.clearDisplay(); // Display the error message
    display.setCursor(0, 0);
    display.println("Verbindungsfehler");
    display.display();
    delay(2000);
    if (Blynk.isTokenInvalid()) // Check if the token was invalid
    {
      Serial.println("Invalid token, please check your Blynk Token"); // Debug message
      display.clearDisplay();                                         // Display connection status
      display.setCursor(0, 0);
      display.println("Token");
      display.println("ungueltig");
      display.display();
      delay(5000);
    }
    wifiManager.erase(); // Erase credentials from the filesystem if the connection failed
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Geraet wird"); // Display message about restarting the device
    display.println("neugestartet");
    display.display();
    delay(5000);
    ESP.restart(); // Restart the program, if the connection could not be established
  }
  delay(2000);

  display.clearDisplay(); // Display connection status
  display.setCursor(0, 0);
  display.println("Verbunden");
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
  {
    releasedTime = millis(); // button is released

    long pressDuration = releasedTime - pressedTime;

    if (pressDuration > SHORT_PRESS_TIME) // long press, erase wifimanager and restart
    {
      wifiManager.erase();
      display.setCursor(0, 0);
      display.clearDisplay(); // Display connection status
      display.println("Verbindung");
      display.println("geloescht");
      display.println("Geraet wird"); // Display message about restarting the device
      display.println("neugestartet");
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

  display.clearDisplay(); // Display that the controller is ready for the next coffee
  display.setCursor(0, 0);
  display.println("Bereit");
  display.println("einen Kaffee");
  display.println("zu machen");
  display.display();

  koffeePouring = false;

  tone(BUZZER_PIN, NOTE_C4, 500); // Play a tone to indicate that the coffee was poured
}

BLYNK_WRITE(V0) // Coffee button in blynk app is pressed
{
  int i = param.asInt(); // Get the value of the button,  1 is pour coffee, 0 is do nothing
  if (i == 0 || koffeePouring)
    return;

  pourCoffee();
}
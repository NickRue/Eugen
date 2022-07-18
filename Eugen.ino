#include <string> // Library to use strings

#include <BlynkSimpleEsp8266.h> // Library to use the Blynk-App

#include <WiFiManager.h>                                              // Library to enable wifi controll
WiFiManager wifiManager;                                              // Create a wifi manager object
boolean wifi_connected = false;                                        // Variable to check if wifi is connected
WiFiManagerParameter blynkToken("blynkToken", "Blynk Token", "", 33); // Parameter to set the Blynk Token

#include <SPI.h> // Libraries to controll the LED-Display
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "Servo.h"             // Library to controll the servo
Servo myservo;                 // Create a servo object
#define SERVO_PIN D7           // Data pin for the servo
int startAngle = 150;          // Angle at which the servo starts to turn
int endAngle = 45;             // Angle at which the servo stops to turn
int tempStartAngle = 0;        // Temporary start angle for calibration
int tempEndAngle = 0;          // Temporary end angle for calibration
boolean calibrateStart = true; // If true, the start angle is being calibrated, otherwise the end angle is being calibrated

#include "pitches.h" // Library contains pitches for notes

#include "FileSystemUtilities.h" // Contains methods to use the filesystem
FileSystemUtilities utilities;   // Create a filesystem utilities object

#define BUTTON_PIN D3              // Pin on which to read the button input
const int SHORT_PRESS_TIME = 3000; // 3 seconds is short press
int lastState = HIGH;              // the previous state from the input pin
int currentState;                  // the current reading from the input pin
unsigned long pressedTime = 0;     // the time the button was pressed
unsigned long releasedTime = 0;    // the time the button was released

#define BUZZER_PIN D5 // Pin on which the buzzer reads

#define OLED_RESET 0 // "0" for ESP8266
Adafruit_SSD1306 display(OLED_RESET);

unsigned long lastCoffeeTime = 0; // Time when the last coffee was made
#define COFFEE_TIME_INTERVAL 5000 // Minimum time in milliseconds between two coffees
boolean calibrationMode = false;  // If true, the controller is in calibration mode

void setup()
{
  Serial.begin(9600); // Start the serial communication

  myservo.attach(SERVO_PIN);         // Attach the servo to the pin
  pinMode(BUTTON_PIN, INPUT_PULLUP); // Set the button pin to input with pullup
  pinMode(BUZZER_PIN, OUTPUT);       // Set the buzzer pin to output

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Initialize the OLED-Display
  display.display();                         // display startup animation
  delay(2000);
  if (utilities.init()) // Mount the file system
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

  String tokenString = utilities.readFile(SPIFFS, "/blynkToken.txt"); // Read the token from the file
  if (!tokenString.isEmpty())
  {
    blynkToken.setValue(tokenString.c_str(), 33); // If a token was found, set the token to the html-page input
  }
  wifiManager.addParameter(&blynkToken); // Add the token as parameter to the wifi manager
  wifiManager.setConfigPortalTimeout(120);   // Set the timeout for the wifi manager to 2 minutes

  // Also accessible at IP 192.168.4.1
  wifiManager.autoConnect("Eugen");                                      // Setup the name of the hotspot and connect to wifi
  utilities.writeFile(SPIFFS, "/blynkToken.txt", blynkToken.getValue()); // Write the blynk token to the filesystem

  if(WiFi.status() != WL_CONNECTED)
  {
    wifi_connected = false;
    display.clearDisplay(); // Display the connection status
    display.setCursor(0, 0);
    display.println("Verbindung");
    display.println("fehlge-");
    display.println("schlagen");
    display.display();
  }
  else
  {
    wifi_connected = true;
  
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
  }

  String startAngleString = utilities.readFile(SPIFFS, "/startAngle.txt"); // Read the start angle from the file
  if (!startAngleString.isEmpty())
  {
    startAngle = startAngleString.toInt(); // If a start angle was found, set the start angle to the html-page input
  }

  String endAngleString = utilities.readFile(SPIFFS, "/endAngle.txt"); // Read the end angle from the file
  if (!endAngleString.isEmpty())
  {
    endAngle = endAngleString.toInt(); // If an end angle was found, set the end angle to the html-page input
  }

  myservo.write(startAngle); // Set the servo to the start angle

  displayReadyToPour(); // Display the ready to pour message
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
      display.println("Verbin-");
      display.println("dung");
      display.println("geloescht");
      display.println("Geraet"); // Display message about restarting the device
      display.println("wird neu-");
      display.println("gestartet");
      display.display();
      delay(5000);
      ESP.restart();
    }
    else
      pourCoffee(); // short press, pour coffee
  }
  lastState = currentState;

  if(wifi_connected)
    Blynk.run(); // Run the Blynk-Client
}

void pourCoffee()
{
  if (calibrationMode)
    return; // If the controller is in calibration mode, do not make a coffee

  if ((millis() - lastCoffeeTime) < COFFEE_TIME_INTERVAL) // If the last coffee was less than COFFEE_TIME_INTERVAL ago, do nothing
    return;

  display.clearDisplay(); // Display that a coffee is beeing poured
  display.setCursor(0, 0);
  display.println("Offline");
  display.println("Mache");
  display.println("Kaffee");
  display.display();

  myservo.write(endAngle); // Rotate the servo so that it presses the button on the coffee machine
  delay(1000);
  myservo.write(startAngle);
  delay(1000);

  displayReadyToPour(); // Display the status that the coffee is ready to pour

  lastCoffeeTime = millis();

  tone(BUZZER_PIN, NOTE_C4, 500); // Play a tone to indicate that the coffee was poured
}

void displayReadyToPour()
{
  display.clearDisplay(); // Display that the controller is ready for the next coffee
  display.setCursor(0, 0);
  if(!wifi_connected)
    display.println("Offline");
    
  display.println("Bereit");
  display.println("einen");
  display.println("Kaffee");
  display.println("zu machen");
  display.display();
}

BLYNK_WRITE(V0) // Coffee button in blynk app is pressed
{
  int i = param.asInt(); // Get the value of the button,  1 is pour coffee, 0 is do nothing
  if (i == 0)
    return;

  pourCoffee();
}

BLYNK_WRITE(V1) // Toogle calibration mode
{
  int i = param.asInt();
  if (i == 1)
  {
    calibrationMode = true;                                  // Activate calibration mode
    Blynk.virtualWrite(V2, startAngle);                      // Set the slider to the current start angle value
    calibrateStart = true;                                   // Set the calibration mode to start
    Blynk.setProperty(V3, "onLabel", "Accept Start Angle");  // Set the on label of the button to "Accept Start Angle"
    Blynk.setProperty(V3, "offLabel", "Accept Start Angle"); // Set the off label of the button to "Accept Start Angle"
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Kalibrie-"); // Display message about calibration mode
    display.println("rung");
    display.display();
  }
  else
  {
    calibrationMode = false;   // Deactivate calibration mode
    myservo.write(startAngle); // Set the servo to the current start angle value
    displayReadyToPour();      // Display the status that the coffee is ready to pour
  }
}

BLYNK_WRITE(V2) // Calibrate the servo
{
  if (!calibrationMode) // If the controller is not in calibration mode, do nothing
    return;

  int i = param.asInt(); // Get the value of the slider, 0 is the minimum, 180 is the maximum
  myservo.write(i);      // Set the servo to the value of the slider
  if (calibrateStart)
    tempStartAngle = i; // If calibrateStart is true, set the start angle to the current angle
  else
    tempEndAngle = i; // If calibrateStart is false, set the end angle to the current angle
}

BLYNK_WRITE(V3) // Save calibration
{
  if (!calibrationMode) // If the controller is not in calibration mode, do nothing
    return;

  int i = param.asInt(); // Get the value of the button,  1 is accept, 0 is do nothing
  if (i == 0)
    return;

  if (calibrateStart)
  {
    startAngle = tempStartAngle;                                                        // Set the start angle to the current angle
    utilities.writeFile(SPIFFS, "/startAngle.txt", std::to_string(startAngle).c_str()); // Write the start angle to the filesystem
    calibrateStart = false;                                                             // Set calibrateStart to false
    Blynk.virtualWrite(V2, endAngle);                                                   // Set the slider to the end angle
    Blynk.setProperty(V3, "onLabel", "Accept End Angle");                               // Set the on label of the button to "Accept End Angle"
    Blynk.setProperty(V3, "offLabel", "Accept End Angle");                              // Set the off lsabel of the button to "Accept End Angle"
  }
  else
  {
    endAngle = tempEndAngle;                                                        // Set the end angle to the current angle
    utilities.writeFile(SPIFFS, "/endAngle.txt", std::to_string(endAngle).c_str()); // Write the end angle to the filesystem
    calibrateStart = true;                                                          // Set calibrateStart to true
    Blynk.virtualWrite(V2, startAngle);                                             // Set the slider to the start angle
    Blynk.setProperty(V3, "onLabel", "Accept Start Angle");                         // Set the on label of the button to "Accept Start Angle"
    Blynk.setProperty(V3, "offLabel", "Accept Start Angle");                        // Set the off label of the button to "Accept Start Angle"
  }
}
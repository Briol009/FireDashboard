//Arduino script for presence detection and person counting, with cloud website reporting
//Majorly contributed by chatGPT

//Arduino is written in their own special C Plus Plus code, which is different from Python in many ways, but for reading, here are major differences
//1. Comments use the //
//2. each line ends with a semicolon ;
//3. Python allows us to be like 'variable = number', C++ needs us to explicitly say what the variable type is in front: 'float variable = number'
//    This is why we will see a lot of blue in front of names of variables
//4. The code has 3 sections: Name variables, setup, loop, functions
//    Name variables: This is where we name everything that is going to be used. This is done in advance. So, like the name of our WIFI, the number of how many people in the room, which 'pin' our sensors are connected to in real life, etc.
//    setup: this is code that runs one single time. Things like turning on the wifi, telling the computer to start talking to the sensors
//    loop: this is code that runs repeatedly forever. This is where it is constantly talking to sensors and getting information, and making the website available for people to see
//    function: this is the same as Python functions, they're just at the bottom of the code instead of top. This is like jumping to the index of a book whenever we see the words we need to know


// Include necessary libraries
#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>
#include "arduino_secrets.h" // Wi-Fi credentials

// Existing includes
#include <Arduino.h>

// Force Sensitive Resistor (FSR) setup
const int FSR_PIN_1 = A0;
const int FSR_PIN_2 = A1;
const float FORCE_THRESHOLD = 250.0;
const unsigned long SEQUENCE_TIMEOUT = 2000;
volatile int counter = 0;

// Partial Infrared (PIR) setup
const int PIR_PIN = 2;
const int LED_PIN = 13;

// Variables for cloud properties
int count;
bool presence;

// Function declarations
void initProperties();
bool isPersonOnSensor(int pin);
float readForce(int pin);
void handleSensorTrigger(int sensorNumber);

// Additional variables and constants
const unsigned long DEBOUNCE_DELAY = 500;
const float VCC = 4.98;
const float R_DIV = 3230.0;
enum Direction {
  NONE,
  ENTERING,
  EXITING
};
unsigned long lastTriggerTime = 0;
Direction lastDirection = NONE;
int lastSensor = 0;
volatile int pirState = LOW;
volatile int val = 0;

// Initialize the cloud connection handler
WiFiConnectionHandler ArduinoIoTPreferredConnection(SECRET_SSID, SECRET_PASS);

void setup() {
  Serial.begin(9600);
  delay(1500);

  // Initialize FSR and PIR sensors
  pinMode(FSR_PIN_1, INPUT);
  pinMode(FSR_PIN_2, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize cloud properties
  initProperties();

  // Connect to Arduino IoT Cloud
  ArduinoCloud.begin(ArduinoIoTPreferredConnection);

  // Optional: Set debug message level
  setDebugMessageLevel(2);
  ArduinoCloud.printDebugInfo();
}

void loop() {
  ArduinoCloud.update();

  // People counting logic
  unsigned long currentTime = millis();
  bool sensor1Active = isPersonOnSensor(FSR_PIN_1);
  bool sensor2Active = isPersonOnSensor(FSR_PIN_2);

  if (sensor1Active) {
    handleSensorTrigger(1);
    delay(DEBOUNCE_DELAY);
  }
  if (sensor2Active) {
    handleSensorTrigger(2);
    delay(DEBOUNCE_DELAY);
  }

  // PIR Presence Detection
  val = digitalRead(PIR_PIN);

  if (val == HIGH) {
    if (pirState == LOW) {
      Serial.println("Motion detected!");
      digitalWrite(LED_PIN, HIGH);
      pirState = HIGH;
    }
  } else {
    if (pirState == HIGH){
      Serial.println("Motion ended!");
      digitalWrite(LED_PIN, LOW);
      pirState = LOW;
    }
  }
  // Update cloud variables
  count = counter;
  presence = pirState == HIGH;
}

// Initialize cloud properties
void initProperties() {
  ArduinoCloud.setThingId("8885012c-36b9-4dd0-888b-846abf8b923b");
  ArduinoCloud.addProperty(count, READ, ON_CHANGE, NULL);
  ArduinoCloud.addProperty(presence, READ, ON_CHANGE, NULL);
}

// Function to check if a person is on the sensor
bool isPersonOnSensor(int pin) {
  float force = readForce(pin);
  return force > FORCE_THRESHOLD;
}

// Function to read force from the sensor
float readForce(int pin) {
  int fsrADC = analogRead(pin);
  if (fsrADC == 0) return 0.0; // No pressure detected
  float fsrV = fsrADC * VCC / 1023.0;
  if (fsrV == 0) return 0.0;
  float fsrR = R_DIV * (VCC / fsrV - 1.0);
  float fsrG = 1.0 / fsrR;
  float force;
  if (fsrR <= 600) 
    force = (fsrG - 0.00075) / 0.00000032639;
  else
    force = fsrG / 0.000000642857;
  return force;
}

// Function to handle sensor triggers
void handleSensorTrigger(int sensorNumber) {
  unsigned long currentTime = millis();
  if (lastSensor == 0) {
    lastSensor = sensorNumber;
    lastTriggerTime = currentTime;
  } else {
    if ((currentTime - lastTriggerTime) <= SEQUENCE_TIMEOUT) {
      if (lastSensor == 1 && sensorNumber == 2) {
        counter++;
        lastDirection = ENTERING;
        Serial.println("Person Entered");
      } else if (lastSensor == 2 && sensorNumber == 1) {
        counter--;
        lastDirection = EXITING;
        Serial.println("Person Exited");
      } else {
        Serial.println("Invalid Sensor Sequence Detected");
      }
    } else {
      Serial.println("Sensor Sequence Timeout");
    }
    lastSensor = 0;
    lastTriggerTime = 0;
  }
  Serial.print("Current Count: ");
  Serial.println(counter);
}

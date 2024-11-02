// #include "WiFi.h"
#include "HardwareSerial.h"
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include "Wifi_connect.cpp"
#define MACHINE_ID "1"
WiFi_connection wifi;
//WIFI and websockets setup
using namespace websockets;
WebsocketsClient webSocket;
const char* ssid = "";        
const char* password = "";

//QR code scannner setup
#define RXD2 16 
#define TXD2 17  
HardwareSerial MySerial(1);  
#define FlatPos 175
#define SkewPos 135
//LCD display setup
LiquidCrystal_I2C lcd(0x27, 20, 4); 

//Servo setup
Servo myservo;  
#define servoPin 26 

// Limit switch setup
#define yesSwitchPin 12 
#define noSwitchPin 13 

String previous_boxId = "";
void sendToServer(String messageType, String boxId, String userId) {
  String messageToSend = "{\"machineId\": \"";
  messageToSend +=MACHINE_ID; 
  messageToSend +="\" , \"messageType\": \"";
  messageToSend += messageType;
  messageToSend += "\", \"boxId\": \"";
  messageToSend += boxId;
  messageToSend += "\", \"userId\": \"";
  messageToSend += userId;
  messageToSend += "\", \"status\":\"\"}";

  Serial.println("Sending message: ");
  Serial.println(messageToSend);
  
  webSocket.send(messageToSend);
}

void displayMessage(const String& message) {
  lcd.clear();
  int messageLength = message.length();
  int displayWidth = 20;
  int startPosition = (displayWidth - messageLength) / 2;
  lcd.setCursor(startPosition, 1);
  lcd.print(message);
}

void returnToInitialScreen() {
  delay(3000);
  lcd.clear();
  Serial.println("Scan QR code");
  displayMessage("Scan QR code");
}

void onMessageCallback(WebsocketsMessage message) {
  previous_boxId = "";
  DynamicJsonDocument doc(1024);
  const char* boxId;
  const char* userId;
  const char* messageType;
  const char* useremail;
  int status; 
  String messageToSend;

  // Parse JSON payload
  deserializeJson(doc, message.data());
  Serial.println("Received message: ");
  serializeJsonPretty(doc, Serial);
  boxId = doc["boxId"];
  useremail = doc["email"];
  userId = doc["userId"];
  messageType = doc["messageType"];
  status = doc["status"];

  // Prepare JSON message to send back
  if (String(messageType) == "query") {
    if (status == 1) {
      displayMessage("User found:");
      lcd.setCursor(0, 2);
      lcd.print(userId);
      delay(3000);

      displayMessage("Return the box?");
      lcd.setCursor(0, 2);
      lcd.print("No               Yes");

      Serial.println("Return the box?");
      while (true) {
        if (digitalRead(yesSwitchPin) == LOW) {
          sendToServer("return", boxId, userId);
          break;
        } else if (digitalRead(noSwitchPin) == LOW) {
          Serial.println("Return canceled");
          displayMessage("Return canceled");
          returnToInitialScreen();
          break;
        }
      }
    } else {
      Serial.println("Box not found"); 
      displayMessage("Box not found");
      returnToInitialScreen();
    }
  } else if (String(messageType) == "return") {
    if (status == 1) {
      Serial.println("Box returned");
      displayMessage("Box returned");
      myservo.write(SkewPos); 
      delay(3000); 
      myservo.write(FlatPos);
      delay(1000); 
      returnToInitialScreen();
    } else {
      Serial.println("Error, please contact service");
      displayMessage("Error, please contact service");
      returnToInitialScreen();
    }
  }
}

void setup() {
  Serial.begin(115200);

  //LCD display
  lcd.init();
  lcd.backlight();
  displayMessage("Connecting...");

  //Wifi
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.println("Connecting to WiFi..");
  // }
  // Serial.println("Connected to the WiFi network");
  wifi.changePEAP("RiceballFan_peap", "brainbrian2000", "brainbrian2000");
  wifi.turn_on_WiFi();
  wifi.turn_on_WiFi(2);
  lcd.clear();

  // Print local IP address
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP()); 

  // connect to WebSocket server
  webSocket.connect("ws://51.20.189.251:8000");
  webSocket.onMessage(onMessageCallback);
  Serial.println("");
  Serial.println("WS connected.");

  // Begin communication with the barcode scanner
  MySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);  

  displayMessage("Scan QR code");

  //Servo
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  
  myservo.setPeriodHertz(50);   
  myservo.attach(servoPin, 500, 2500);
  myservo.write(FlatPos);

  // Limit switch
  pinMode(yesSwitchPin, INPUT_PULLUP); 
  pinMode(noSwitchPin, INPUT_PULLUP); 
}
void loop() {
  webSocket.poll();
  if (MySerial.available()) {  // If data is available to read,
    Serial.println("Read:");
    while (MySerial.available())
    {
      String boxId = MySerial.readString();  
      Serial.println(boxId);  
      if (boxId.equals(previous_boxId) == false) {
        if (boxId != ""){
          previous_boxId = boxId;
          sendToServer("query", boxId, " ");
        }
      }else{
        Serial.println("Same boxId");
      }
    }
  }
}
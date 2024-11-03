// #include "WiFi.h"
#include "HardwareSerial.h"
#include <ArduinoWebsockets.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include "Wifi_connect.cpp"
WiFi_connection wifi;
//WIFI and websockets setup
using namespace websockets;
WebsocketsClient webSocket;
const char* ssid = "";        
const char* password = "";
// static SemaphoreHandle_t xSemaphore;
//QR code scannner setup
#define RXD2 16 
#define TXD2 17  
HardwareSerial MySerial(1);  
#define MACHINE_ID "5"
#define FlatPos 145
#define SkewPos 118
#define WiFitest 0
//LCD display setup
LiquidCrystal_I2C lcd(0x27, 20, 4); 
#define yesSwitchPin 12 
#define noSwitchPin 13 

//Servo setup
Servo myservo;  
// I2C scl 22 sda 21
// Limit switch setup
#define servoPin 26 
String previous_boxId = "";
String messageToSend = "";
void sendToServer(String messageType, String boxId, String userId) {
  messageToSend = "";
  messageToSend = "{\"machineId\": \"";
  messageToSend +=MACHINE_ID; 
  messageToSend +="\" , \"messageType\": \"";
  messageToSend += messageType;
  messageToSend += "\", \"boxId\": \"";
  messageToSend += boxId;
  messageToSend += "\", \"userId\": \"";
  messageToSend += userId;
  messageToSend += "\", \"status\":\"\"}";
  char messageToSendChar[120];
  strcpy(messageToSendChar, messageToSend.c_str());
  Serial.println("Sending message: ");
  Serial.println(messageToSendChar);
  vTaskDelay(100);
  // xSemaphoreTake(xSemaphore, portMAX_DELAY);
  webSocket.send(messageToSend);
  // xSemaphoreGive(xSemaphore);
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
      lcd.print(useremail);
      delay(3000);

      displayMessage("Return the box?");
      lcd.setCursor(0, 2);
      // lcd.print("No               Yes");
      lcd.print("Yes               No");
      Serial.println("Return the box?");
      while (true) {
        if (digitalRead(yesSwitchPin) == LOW) {
          sendToServer("return", boxId, userId);
          previous_boxId = "";
          break;
        } else if (digitalRead(noSwitchPin) == LOW) {
          Serial.println("Return canceled");
          displayMessage("Return canceled");
          returnToInitialScreen();
          previous_boxId = "";
          break;
        }
      }
    } else {
      Serial.println("Box not found"); 
      displayMessage("Box not found");
      // previous_boxId = "";
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

String boxId = "";
void SerialReadTask(void *param){
  while(1){
    if (MySerial.available()) {  // If data is available to read,
    boxId = "";
      while (MySerial.available())
      {
        boxId+= (char)MySerial.read();
      }
        // boxId = MySerial.readString();  
        
        if (boxId.equals(previous_boxId) == false) {
          // Serial.println("Read:");
          // Serial.println(boxId);  
          // find boxId \r and \n and remove them
          // Serial
          if (boxId != ""){
            previous_boxId = boxId;
            boxId.replace("\r", "");
            boxId.replace("\n", "");
            Serial.printf("boxID in c str:%8s ,len%d\n",boxId.c_str(),strlen(boxId.c_str()));
            sendToServer("query", boxId, " ");
          }
        }else{
          // Serial.println("Same boxId");
        }
    }
    vTaskDelay(pdMS_TO_TICKS(300));
  }
};
void setup(){
  Serial.begin(115200);
  // xSemaphore = xSemaphoreCreateMutex();
  myservo.setPeriodHertz(50);   
  myservo.attach(servoPin, 500, 2500);
  myservo.write(FlatPos);
  Serial.printf("\nThis is machine %s\n",MACHINE_ID);
  vTaskDelay(2000);
  myservo.write(SkewPos);
  vTaskDelay(2000);
  myservo.write(FlatPos);
  //LCD display
  lcd.init();
  lcd.backlight();
  displayMessage("Connecting...");
  xTaskCreate(SerialReadTask, "SerialReadTask", 4096, NULL, 19, NULL);
  //Wifi
  // WiFi.begin(ssid, password);
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.println("Connecting to WiFi..");
  // }
  // Serial.println("Connected to the WiFi network");
  // wifi.changeWPA2("makerspace-2.4G","ntueemakerspace");
  // wifi.changeWPA2("RiceballFan","brainbrian2000");
  wifi.turn_on_WiFi();
  // wifi.turn_on_WiFi(1);
  #if WiFitest
    wifi.changePEAP("RiceballFan_peap", "brainbrian2000", "brainbrian2000");
  #else
    wifi.changePEAP("eduroam", "r13247001@eduroam.ntu.edu.tw", "RiceballFan2000");
  #endif
  wifi.turn_on_WiFi(2);
  // wifi.PingTest();
  lcd.clear();

  // Print local IP address
  // Serial.println("IP address: ");
  // Serial.println(WiFi.localIP()); 

  // connect to WebSocket server
  // xSemaphoreTake(xSemaphore, portMAX_DELAY);
  webSocket.connect("ws://54.178.141.189:8000");
  webSocket.onMessage(onMessageCallback);
  // xSemaphoreGive(xSemaphore);
  Serial.println("");
  Serial.println("WS connected.");
  
  // Begin communication with the barcode scanner
  MySerial.begin(9600, SERIAL_8N1, RXD2, TXD2);  
  if(wifi.PingTest()){
#define SkewPos 135
    displayMessage("Scan QR code");
  }else{
    displayMessage("No WiFi");
  }


  //Servo
  // Allow allocation of all timers
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);
  

  // Limit switch
  pinMode(yesSwitchPin, INPUT_PULLUP); 
  pinMode(noSwitchPin, INPUT_PULLUP); 
}
void loop() {
  // xSemaphoreTake(xSemaphore, portMAX_DELAY);
  webSocket.poll();
  // xSemaphoreGive(xSemaphore);
  // Serial.println("polling websocket");
  // if (MySerial.available()) {  // If data is available to read,
  //   Serial.println("Read:");
  //   while (MySerial.available())
  //   {
  //     String boxId = MySerial.readString();  
  //     Serial.println(boxId);  
  //     if (boxId.equals(previous_boxId) == false) {
  //       if (boxId != ""){
  //         previous_boxId = boxId;
  //         sendToServer("query", boxId, " ");
  //       }
  //     }else{
  //       Serial.println("Same boxId");
  //     }
  //   }
  // }
}
/*
 inspired by Rui Santos Complete project details at https://RandomNerdTutorials.com/telegram-esp32-cam-photo-arduino/
*/

#include <ESP8266WiFi.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <SparkFun_SCD30_Arduino_Library.h>
#include <Wire.h>

const char* ssid = "";
const char* password = "";

// Initialize Telegram BOT
String BOTtoken = "";  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
String CHAT_ID = "";

WiFiClientSecure clientTCP;
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
UniversalTelegramBot bot(BOTtoken, clientTCP);


//Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

bool redState = LOW;
bool greenState = LOW;

int CO2 = 0 ;
//Reading CO2, humidity and temperature from the SCD30 By: Nathan Seidle SparkFun Electronics 

//https://github.com/sparkfun/SparkFun_SCD30_Arduino_Library

SCD30 airSensorSCD30; // Objekt SDC30 Umweltsensor
String Unsinn = "" ;
// -----------  Einlesefunktion String von serieller Schnittstelle
String SerialReadEndless(String Ausgabe) { 
  String ret;
  Serial.print(Ausgabe);
  Serial.setTimeout(2147483647UL); // warte bis Eingabe
  ret = Serial.readStringUntil('\n');
  return ret;
}
void handleNewMessages(int numNewMessages) {
  Serial.print("Handle New Messages: ");
  Serial.println(numNewMessages);

  for (int i = 0; i < numNewMessages; i++) {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    
    String from_name = bot.messages[i].from_name;
    if (text == "/start") {
      String welcome = "Welcome , " + from_name + "\n";
      welcome += "Use the following commands to interact with the CO2 Sensor \n";
      welcome += "/co2 : get the sensor readings\n";
      welcome += "/green : switch green LED\n";
      welcome += "/red : switch red LED\n";
      bot.sendMessage(chat_id, welcome, "");
    }
    if (text == "/green") {
      greenState = !greenState;
      digitalWrite(14, greenState);
      Serial.println("Change green LED state");
      String response ="Changed green LED state";
      bot.sendMessage(chat_id,response);
    }
    if (text == "/red") {
      redState = !redState;
      digitalWrite(12, redState);
      Serial.println("Change red LED state");
      String response ="Changed red LED state";
      bot.sendMessage(chat_id,response);
    }
    if (text == "/co2") {
      String response = "Current CO2 reading is: "+String(String(CO2))+ "\n";
      bot.sendMessage(chat_id, response);
    }
  }
}


void setup(){
  // Init Serial Monitor
  Serial.begin(115200);
  Wire.begin(); // ---- Initialisiere den I2C-Bus 

  if (Wire.status() != I2C_OK) Serial.println("Something wrong with I2C");

  if (airSensorSCD30.begin() == false) {
    Serial.println("The SCD30 did not respond. Please check wiring."); 
    while(1) {
      yield(); 
      delay(1);
    } 
  }

  airSensorSCD30.setAutoSelfCalibration(true); // Sensirion no auto calibration
  airSensorSCD30.setMeasurementInterval(2);     // CO2-Messung alle 5 s

  Serial.println();
  pinMode( 12 , OUTPUT);
  pinMode( 14 , OUTPUT);
  pinMode( 13 , OUTPUT);

  // Connect to Wi-Fi  
  WiFi.mode(WIFI_STA);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  clientTCP.setTrustAnchors(&cert); // Add root certificate for api.telegram.org 
  const uint8_t fingerprint[20] = { 0xBB, 0xDC, 0x45, 0x2A, 0x07, 0xE3, 0x4A, 0x71, 0x33, 0x40, 0x32, 0xDA, 0xBE, 0x81, 0xF7, 0x72, 0x6F, 0x4A, 0x2B, 0x6B };
  clientTCP.setFingerprint(fingerprint);
  clientTCP.setInsecure();
}


void loop() {
  CO2 = airSensorSCD30.getCO2() ;
  if (CO2 > 600) {
          String message =  "Current CO2 reading is: "+String(String(CO2)) + "\n"+"Time to open the windows!!";
          bot.sendMessage(CHAT_ID, message);
  }
  Serial.println("CO2="+String(String(CO2)));
  if (millis() > lastTimeBotRan + botRequestDelay)  {
    Serial.println("now check for new messages");
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      Serial.println("got response");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
  delay(2000);
}

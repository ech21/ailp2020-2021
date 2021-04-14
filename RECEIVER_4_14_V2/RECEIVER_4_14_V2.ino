/*
  ----RECEIVER----
  4/14/2021
  Created by Ethan Hui
  MOMI Project Conductivity and Temperature Sensor Code
  Sensors:
  Atlas Scientific K 0.1 Conductivity Sensor
  Atlas Scientific PT-1000 Temperature Sensor
  This code enables the sensors to take readings and send them to the Ubidots IoT API for storage.
*/


//Heltec Board Manager URL: https://resource.heltec.cn/download/package_heltec_esp32_index.json



/****************************************
   Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>  //https://github.com/knolleary/pubsubclient
#include <Wire.h>
#include <SPI.h>
#include <LoRa.h>
#include <Adafruit_GFX.h>
//#include <Adafruit_BusIO.h>
/*
   Adafruit_GFX needs two libraries
   https://github.com/adafruit/Adafruit-GFX-Library
   https://github.com/adafruit/Adafruit_BusIO
*/
#include <Adafruit_SSD1306.h> //https://github.com/adafruit/Adafruit_SSD1306

/****************************************
   Wifi Credentials
 ****************************************/

#define WIFISSID "roboticswifi" // Put your WifiSSID here
#define PASSWORD "scil3ehawk" // Put your wifi password here
#define TOKEN "BBFF-feRGGCQB0EUbp3ui7yPHy8ywkLRPTf" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "f2y363h6x0a8" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
//it should be a random and unique ascii string and different from all other devices

/****************************************
   Define Constants
 ****************************************/
#define TempLabel "RTD" // Assign the variable label
#define DEVICE_LABEL "esp32" // Assign the device label
#define ECLabel "EC"

//define pins used by the transceiver
#define SCK 5
#define MISO 19
#define MOSI 27
#define SS 18
#define RST 14
#define DIO0 26
#define BAND 915E6


//OLED pins
#define OLED_SDA 4
#define OLED_SCL 15
#define OLED_RST 16
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RST);


char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char topic[150];
WiFiClient ubidots;
PubSubClient client(ubidots);

// Space to store values to send

String LoRaData;
int counter;
char temperatureSend[10];

void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.println("Attempting MQTT connection...");
    
    // Attemp to connect
    if (client.connect(MQTT_CLIENT_NAME, TOKEN, "")) {
      Serial.println("Connected");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 2 seconds");
      // Wait 2 seconds before retrying
      delay(2000);
    }
  }
}
void setup() {

  Serial.begin(115200);

  // Assign the pin as INPUT
  WiFi.begin(WIFISSID, PASSWORD);
  pinMode(OLED_RST, OUTPUT);
  digitalWrite(OLED_RST, LOW);
  delay(20);
  digitalWrite(OLED_RST, HIGH);

  Wire.begin(OLED_SDA, OLED_SCL);
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3c, false, false)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  Serial.println();
  Serial.print("Wait for WiFi...");
  
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  
  Serial.println("");
  Serial.println("WiFi Connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);  

  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("LORA RECEIVER ");
  display.display();

  //initialize Serial Monitor
  Serial.begin(115200);

  Serial.println("LoRa Sender Test");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initializing OK!");
  display.setCursor(0, 10);
  display.print("LoRa Initializing OK!");
  display.display();
  delay(2000);



}

void loop() {
   if (!client.connected()) {
    reconnect();
  }


  //try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    //received a packet
    Serial.print("Received packet ");

    //read packet
    while (LoRa.available()) {
      LoRaData = LoRa.readString();
      Serial.print(LoRaData);
    }

    //print RSSI of packet
    int rssi = LoRa.packetRssi();
    Serial.print(" with RSSI ");
    Serial.println(rssi);

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("LORA RECEIVER");
    display.setCursor(0, 20);
    display.setTextSize(1);
    display.print("Received:  ");
    display.print(LoRaData);
    display.setCursor(0, 30);
    display.print("counter   ");
    display.setCursor(50, 30);
    display.print(counter);
    display.display();
    counter ++;

    sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
    sprintf(payload, "%s", ""); // Cleans the payload
    sprintf(payload, "{\"%s\":", TempLabel); // Adds the variable label
    /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
    //dtostrf(LoRaData, 4, 2, temperatureSend);
    sprintf(payload, "%s {\"value\": %s}}", payload, LoRaData); // Adds the value
    Serial.println("Publishing temperature data to Ubidots Cloud");
    client.publish(topic, payload);
    client.loop();
    delay(2000);

    /*
      sprintf(payload, "%s", ""); // Cleans the payload
      sprintf(payload, "{\"%s\":", ECLabel); // Adds the variable label
      dtostrf(conductivity, 4, 2, conductivitySend);
      sprintf(payload, "%s {\"value\": %s}}", payload, conductivitySend); // Adds the value
      Serial.println("Publishing conductivity data to the Ubidots Cloud");
      client.publish(topic,payload);


      client.loop();
    */
    



  }
}

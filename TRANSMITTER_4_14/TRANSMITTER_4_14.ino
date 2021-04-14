 
/*
  ----TRANSMITTER----
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

#include <Wire.h>
#include <Ezo_i2c.h> // https://www.whiteboxes.ch/docs/tentacle/t2-mkII/#/continuous-example

#include <SPI.h>
#include <LoRa.h> //https://github.com/sandeepmistry/arduino-LoRa

#include <Adafruit_GFX.h>
//#include <Adafruit_BusIO.h>
/*
 * Adafruit_GFX needs two libraries
 * https://github.com/adafruit/Adafruit-GFX-Library
 * https://github.com/adafruit/Adafruit_BusIO
 */



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


Ezo_board rtd = Ezo_board(102, "TEMP");
Ezo_board ec = Ezo_board(100, "EC");
float temperature;
char temperatureSend[10];
float conductivity;
char conductivitySend[10];


void setup() {

  Serial.begin(115200);
  Wire.begin();
 
  
  
  //initialize Serial Monitor
  
  Serial.println("LoRa Sender Test");

  //SPI LoRa pins
  SPI.begin(SCK, MISO, MOSI, SS);
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);
  
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
 

}

void loop() {
  
  rtd.send_read_cmd();
  delay(1500); 
  receive_reading(rtd, 102);
  delay(2000); 
  ec.send_read_cmd();
  delay(1500);
  receive_reading(ec, 100);

  LoRa.beginPacket();
  LoRa.print(temperature);
  LoRa.endPacket();

} 
void receive_reading(Ezo_board &Sensor, int index) {

  Serial.print(Sensor.get_name()); Serial.print(": ");  // print the name of the circuit getting the reading
  Sensor.receive_read_cmd();                            // get the response data

  switch (Sensor.get_error()) {                         // switch case based on what the response code is.
    case Ezo_board::SUCCESS:
      Serial.println(Sensor.get_last_received_reading()); //the command was successful, print the reading
      if (index == 102) {
        temperature = Sensor.get_last_received_reading();
      } else if (index == 100) {
        conductivity = Sensor.get_last_received_reading();
      }
      break;

    case Ezo_board::FAIL:
      Serial.println("Failed ");                          //means the command has failed.
      break;

    case Ezo_board::NOT_READY:
      Serial.println("Pending ");                         //the command has not yet been finished calculating.
      break;

    case Ezo_board::NO_DATA:
      Serial.println("No Data ");                         //the sensor has no data to send.
      break;
  }
}

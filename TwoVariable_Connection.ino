/****************************************
   Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>

#include <Wire.h>
#include <Ezo_i2c.h>
#define WIFISSID "roboticswifi" // Put your WifiSSID here
#define PASSWORD "scil3ehawk" // Put your wifi password here
#define TOKEN "BBFF-feRGGCQB0EUbp3ui7yPHy8ywkLRPTf" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "f2y363h6x0a8" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
//it should be a random and unique ascii string and different from all other devices

/****************************************
   Define Constants
 ****************************************/
#define TempLabel "RTD" // Assign the variable label
#define DEVICE_LABELrtd "esp32rtd" // Assign the device label

#define ECLabel "EC"


char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char topic[150];
// Space to store values to send

Ezo_board rtd = Ezo_board(102, "TEMP");
Ezo_board ec = Ezo_board(100, "EC");
float temperature;
char temperatureSend[10];
float conductivity;
char conductivitySend[10];
/****************************************
   Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);


void callback(char* topic, byte* payload, unsigned int length) {
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.print("Payload = ");
  //  Serial.println(payload);
  Serial.print("Topic = ");
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

/****************************************
   Main Functions
 ****************************************/
void setup() {
  Wire.begin();
  Serial.begin(115200);
  WiFi.begin(WIFISSID, PASSWORD);
  // Assign the pin as INPUT


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

}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  rtd.send_read_cmd();
  delay(15000); 
  receive_reading(rtd, 102);
  delay(2000); 
  ec.send_read_cmd();
  delay(15000);
 
  receive_reading(ec, 100);

  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABELrtd);
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", TempLabel); // Adds the variable label
  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
  dtostrf(temperature, 4, 2, temperatureSend);
  sprintf(payload, "%s {\"value\": %s}}", payload, temperatureSend); // Adds the value
  Serial.println("Publishing temperature data to Ubidots Cloud");
  client.publish(topic, payload);
  client.loop();
  delay(2000);
  
  sprintf(payload, "%s", ""); // Cleans the payload
  sprintf(payload, "{\"%s\":", DEVICE_LABELrtd); // Adds the variable label
  dtostrf(conductivity, 4, 2, conductivitySend);
  sprintf(payload, "%s {\"value\": %s}}", payload, conductivitySend); // Adds the value
  Serial.println("Publishing conductivity data to the Ubidots Cloud");
  client.publish(topic,payload);

  
  client.loop();
  delay(2000); //5 seconds
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

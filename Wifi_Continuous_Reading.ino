/****************************************
 * Include Libraries
 ****************************************/
#include <WiFi.h>
#include <PubSubClient.h>

#include <Ezo_i2c.h> 
#include <Wire.h>

#define WIFISSID "roboticswifi" // Put your WifiSSID here
#define PASSWORD "scil3ehawk" // Put your wifi password here
#define TOKEN "BBFF-feRGGCQB0EUbp3ui7yPHy8ywkLRPTf" // Put your Ubidots' TOKEN
#define MQTT_CLIENT_NAME "f2y363h6x0a8" // MQTT client Name, please enter your own 8-12 alphanumeric character ASCII string; 
                                           //it should be a random and unique ascii string and different from all other devices

Ezo_board rtd = Ezo_board(102, "TEMP");
Ezo_board ec = Ezo_board(100, "EC");
/****************************************
 * Define Constants
 ****************************************/
#define VARIABLE_LABEL_rtd "sensor_rtd" // Assing the variable label
#define DEVICE_LABEL "esp32" // Assig the device label
#define VARIABLE_LABEL_ec "sensor_ec"



char mqttBroker[]  = "industrial.api.ubidots.com";
char payload[100];
char topic[150];
// Space to store values to send
char str_sensor[10];

/****************************************
 * Auxiliar Functions
 ****************************************/
WiFiClient ubidots;
PubSubClient client(ubidots);


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

float reading;
String NAME;

/****************************************
 * Main Functions
 ****************************************/
void setup() {
  Serial.begin(115200);
  Wire.begin();
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
   ec.send_read_cmd();
  rtd.send_read_cmd();
  delay(1000);
  receive_reading(ec);
  receive_reading(rtd);
  Serial.println();

  if(NAME = "EC") {
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL_ec);
  } else {
    sprintf(payload, "{\"%s\":", VARIABLE_LABEL_rtd);
  }
  sprintf(topic, "%s%s", "/v1.6/devices/", DEVICE_LABEL);
  sprintf(payload, "%s", ""); // Cleans the payload
   // Adds the variable label
  
  
  
  
  /* 4 is mininum width, 2 is precision; float value is copied onto str_sensor*/
 
  
  sprintf(payload, "%s {\"value\": %s}}", payload, reading); // Adds the value
  Serial.println("Publishing data to Ubidots Cloud");
  client.publish(topic, payload);
  client.loop();
  delay(30000); //30 seconds
}
void receive_reading(Ezo_board &Sensor) {

  Serial.print(Sensor.get_name()); Serial.print(": ");  // print the name of the circuit getting the reading
  Sensor.receive_read_cmd();                            // get the response data
  NAME = Sensor.get_name();
  switch (Sensor.get_error()) {                         // switch case based on what the response code is.
    case Ezo_board::SUCCESS:
      reading = Sensor.get_last_received_reading();
      Serial.print(Sensor.get_last_received_reading()); //the command was successful, print the reading
      break;

    case Ezo_board::FAIL:
      Serial.print("Failed ");                          //means the command has failed.
      break;

    case Ezo_board::NOT_READY:
      Serial.print("Pending ");                         //the command has not yet been finished calculating.
      break;

    case Ezo_board::NO_DATA:
      Serial.print("No Data ");                         //the sensor has no data to send.
      break;
  }
}

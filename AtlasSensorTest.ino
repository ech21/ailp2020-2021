
#include <Ezo_i2c.h>
#include <Wire.h>

Ezo_board RTD = Ezo_board(1, "RTD");
Ezo_board EC = Ezo_board(2, "EC");

int RTD_LED = 5;
int EC_LED = 6;

bool read_phase = true;

uint32_t next_read = 0;
const unsigned int response_delay = 1000;

void setup() {
  Wire.begin();
  Serial.begin(9600);
  pinMode(RTD_LED, OUTPUT);
  pinMode(EC_LED, OUTPUT);

}

void loop() {
  if(read_phase) {
    RTD.send_read_cmd();
    EC.send_read_cmd();

    next_read = millis() + response_delay;
    
    read_phase = false;
    
  }
  else {
    if(millis() >= next_read){

      get_read(RTD);
      get_read(EC);

      read_phase = true;
      Serial.println();
      
    }
    
  }

}

void get_read(Ezoboard::Sensor){
  if(sensor == "RTD");
  Serial.print("yay");
  
}

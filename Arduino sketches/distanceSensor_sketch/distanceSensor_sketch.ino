#include "Adafruit_VL53L0X.h"

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
int measuring_interval = 2000;
unsigned long timer = 0;

void setup() {
  Serial.begin(115200);

  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
  
  Serial.println("Adafruit VL53L0X test");
  if (!lox.begin()) {
    Serial.println(F("Failed to boot VL53L0X"));
    while(1);
  }
  
  Serial.println(F("VL53L0X API Simple Ranging example\n\n")); 
}


void loop() {
  if(millis() - timer >= measuring_interval){
    timer += measuring_interval;
    Serial.println(get_sensor_measurement());
  }
  
}

uint16_t get_sensor_measurement(){
  VL53L0X_RangingMeasurementData_t measure;
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!
  uint16_t measurement = 0;
  
  if (measure.RangeStatus != 4) {  // phase failures have incorrect data
    measurement = measure.RangeMilliMeter;
  } else {
    return 0;
  }
  
  return measurement;
}

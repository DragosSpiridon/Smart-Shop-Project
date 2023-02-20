#include <SPI.h>
#include <MFRC522.h>
#include "Adafruit_VL53L0X.h"
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>

#define RST_PIN  16         
#define SS_PIN   15         

int measuring_interval = 7000;
unsigned long timer = 0;

const char* ssid = "NDL_24G"; // name of local WiFi network in the NDL
const char* password = "RT-AC66U"; // the password of the WiFi network

//Your Domain name with URL path or IP address with path
String serverName = "http://192.168.1.179:3000";

// Distance sensor relevant variables
Adafruit_VL53L0X lox = Adafruit_VL53L0X();
uint16_t initial_measurement = 0;
uint16_t item_depth = 15;
String item_id = "638b2b1ac6472a765db057eb";

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
MFRC522::MIFARE_Key key;
String uid;

//////////////////////////////////// SET-UP /////////////////////////////////
void setup() {
  Serial.begin(115200);                 // Initialize serial communications with the PC
  while (!Serial);                      // Do nothing if no serial port is opened
  
  SPI.begin();                          // Init SPI bus
  mfrc522.PCD_Init();                   // Init MFRC522
  delay(100);                           // Optional delay. Some board do need more time after init to be ready, see Readme

  // Prepare the key (used both as key A and as key B)
  // using FFFFFFFFFFFFh which is the default at chip delivery from the factory
  for (byte i = 0; i < 6; i++) {
      key.keyByte[i] = 0xFF;
  }
  
  if (!lox.begin()) {
    Serial.println("Failed to boot VL53L0X");
    while(1);
  }

  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());

  while(initial_measurement == 0){
      initial_measurement = get_sensor_measurement();
  }
}

////////////////////////////////////   MAIN LOOP //////////////////////////////////////////////////

void loop() {
  uid = getID();
  if(uid == "") {
    return;
  }
  
  Serial.print("You scanned card with id: "); 
  Serial.println(uid);
  
  if (findID()) { 
    Serial.println("Welcome, You have 7 seconds to pick up an item.");
    int item_count=0;
    // BUYING PROCESS
    
    while(initial_measurement == 0){
      initial_measurement = get_sensor_measurement();
    }
    Serial.print("Initial measurement: "); Serial.println(initial_measurement);
    
    uint16_t measurement;
    
    timer = millis();
    while(millis() - timer <= measuring_interval){
      delay(10);
      measurement = get_sensor_measurement();
      Serial.println(measurement);
      while(measurement == 0){
        measurement = get_sensor_measurement();
      }
      if(measurement <= initial_measurement - item_depth){
        initial_measurement = initial_measurement - item_depth;
        item_count--;
        Serial.print("Item count changed to: "); Serial.println(item_count);
      }
      if(measurement >= initial_measurement + item_depth){
        initial_measurement = initial_measurement + item_depth;
        item_count++;
        Serial.print("Item count changed to: "); Serial.println(item_count);
      }
    }
    if(item_count != 0){
      Serial.print("Item count: "); 
      Serial.println(item_count);
      orderItem(serverName + "/orders",uid,item_id,item_count);
    }
  }
  else {
    Serial.println("You are not a client!");
  }
}

/////////////////////////////////////  SERVER CALLS   /////////////////////////////////////////

bool findID(){
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;

    String serverPath = serverName + "/clients/" + uid;
    
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverPath.c_str());   
           
    // Send HTTP POST request
    int httpResponseCode = http.GET();
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      if(payload == "true"){
        return true;
      } else {
        return false;
      }
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
      return false;
    }
    
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
    return false;
  }
}

void orderItem(String serverPath, String userID, String itemID, int itemCount){
  if(WiFi.status()== WL_CONNECTED){
    WiFiClient client;
    HTTPClient http;
    
    // Your Domain name with URL path or IP address with path
    http.begin(client, serverPath.c_str());
    
    // Specify content-type header
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    
    // Data to send with HTTP POST
    String httpRequestData = "client="+String(userID)+"&item="+String(itemID)+"&quantity="+String(itemCount);    
           
    // Send HTTP POST request
    int httpResponseCode = http.POST(httpRequestData);
    
    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
    } else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    
    // Free resources
    http.end();
  }
  else {
    Serial.println("WiFi Disconnected");
  }
}

//////////////////////////////////////////  HELPER FUNCTIONS   //////////////////////////////////////////

String getID(){
  // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
    if ( ! mfrc522.PICC_IsNewCardPresent())
        return "";
    // Select one of the cards
    if ( ! mfrc522.PICC_ReadCardSerial())
        return "";

    byte sector         = 1;
    byte blockAddr      = 4;
    byte trailerBlock   = 7;
    MFRC522::StatusCode status;
    byte buffer[18];
    byte size = sizeof(buffer);

    // Authenticate using key A
    Serial.println(F("Authenticating using key A..."));
    status = (MFRC522::StatusCode) mfrc522.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A, trailerBlock, &key, &(mfrc522.uid));
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("PCD_Authenticate() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
        return "";
    }

    // Read data from the block
    status = (MFRC522::StatusCode) mfrc522.MIFARE_Read(blockAddr, buffer, &size);
    if (status != MFRC522::STATUS_OK) {
        Serial.print(F("MIFARE_Read() failed: "));
        Serial.println(mfrc522.GetStatusCodeName(status));
    }
    String uid;
    for(byte i = 0; i < 12; i++){
      String next_byte =  String(buffer[i], HEX);
      while(next_byte.length() < 2){
        next_byte = "0" + next_byte;
      }
      uid = uid + next_byte;
    }
    // Halt PICC
    mfrc522.PICC_HaltA();
    // Stop encryption on PCD
    mfrc522.PCD_StopCrypto1();
    return uid;
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

/*
  This program is the interpretation routine of standard output protocol of TFmini-Plus product on Arduino.
  For details, refer to Product Specifications.
  For Arduino boards with only one serial port like UNO board, the function of software visual serial port is
  to be used.
*/

#include <ESP8266WiFi.h>
#include <espnow.h>
#include <BlynkSimpleEsp8266.h>
#include <Arduino.h>     // Every sketch needs this
#include <Wire.h>        // Instantiate the Wire library
#include <TFLI2C.h>  // TFLuna-I2C Library v.0.1.0

TFLI2C tflI2C;



#define TFL_DEF_ADR 0x10

// Set your Board ID (ESP32 Sender #1 = BOARD_ID 1, ESP32 Sender #2 = BOARD_ID 2, etc)
#define BOARD_ID 2

//NodeMCU:--> BC:DD:C2:46:1D:CD

// REPLACE WITH RECEIVER MAC Address
uint8_t broadcastAddress[] = {0xBC, 0xDD, 0xC2, 0x46, 0x1D, 0xCD};

int16_t  tfDist;    // distance in centimeters
int16_t  tfAddr = TFL_DEF_ADR;  // Default I2C address
                                // Set variable to your value
                                
// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  int id;
  int Distance;
  int Signal;
  int Chip_Temp;
} struct_message;

// Create a struct_message called test to store variables to be sent
struct_message myData;

unsigned long lastTime = 0;
unsigned long timerDelay = 1000;


//int dist; //actual distance measurements of LiDAR
int strength = 0; //signal strength of LiDAR
float temprature = 0;
//int check; //save check value
//int i;
//int uart[9]; //save data measured by LiDAR
//const int HEADER = 0x59; //frame header of data package


// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  //Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0) {
    Serial.println("Delivery success");
  }
  else {
    Serial.println("Delivery fail");
  }
}

void setup() {

  Serial.begin(9600); //set bit rate of serial port connecting Arduino with computer
  Wire.begin(0, 2); //Change to Wire.begin() for non ESP.
  delay(2000);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  // Set ESP-NOW role
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);

  // Once ESPNow is successfully init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);

  // Register peer
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

}
void loop() {

   if( tflI2C.getData( tfDist, tfAddr)) // If read okay...
    {
        Serial.print("Dist: ");
        Serial.println(tfDist);         // print the data...
    }
    //else { tflI2C.printStatus(); }         // else, print error.

    delay( 50);

  if ((millis() - lastTime) > timerDelay) {
    // Set values to send
    myData.id = BOARD_ID;
    myData.Distance = tfDist;
    //myData.Signal = strength;
    //myData.Chip_Temp = temprature;

    // Send message via ESP-NOW
    esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
    lastTime = millis();
  }
}

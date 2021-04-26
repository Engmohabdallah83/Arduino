
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp-now-many-to-one-esp8266-nodemcu/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// REPLACE WITH RECEIVER MAC Address
//uint8_t broadcastAddress1[] = {0xA0, 0x20, 0xA6, 0x2A, 0x4A, 0x9B};
//uint8_t broadcastAddress2[] = {0xBC, 0xDD, 0xC2, 0x46, 0x1D, 0xCD};

#include<ESP8266WiFi.h>
#include<espnow.h>

#include<BlynkSimpleEsp8266.h>


char auth[] = "m-B81niVl9VdiglxK-h8flNefcRd_B8-";

char ssid[] = "Mohamed HUAWEI P9";
char pass[] = "asd@12345";


///////////////////////////////////////////////

unsigned char Flag = 0;
///////////////////////////////////////////////

///////////////////////////////////
WidgetLCD WLCD(V0);

WidgetLED led1(V1); //register to virtual pin 1
///////////////////////////////////


// Structure example to receive data
// Must match the sender structuretypedef struct struct_message {
typedef struct struct_message {
    int id;
    int x;
    int y;
    int Chip_Temp;
} struct_message;
// Create a struct_message called myData
struct_message myData;

// Create a structure to hold the readings from each board
struct_message board1;
struct_message board2;

// Create an array with all the structures
struct_message boardsStruct[2] = {board1, board2};

// Callback function that will be executed when data is received
void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) 
{
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].x = myData.x;
  boardsStruct[myData.id-1].y = myData.y;
  boardsStruct[myData.id-1].Chip_Temp = myData.Chip_Temp;
  Serial.printf("Distance value: %d \n", boardsStruct[myData.id-1].x);
  Serial.printf("Signal value: %d \n", boardsStruct[myData.id-1].y);
  //Serial.printf("Chip_Temp value: %d \n", boardsStruct[myData.id-1].Chip_Temp);
  Serial.println();

  Flag = 0; //This flag for confirm that the recieved value is new not the old one
}
 
void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  
  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_AP_STA);
  WiFi.disconnect();

  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop(){
  // Access the variables for each board
  //int board1X = boardsStruct[0].x;
  //int board1Y = boardsStruct[0].y;
  int board2X = boardsStruct[myData.id-1].x;
  int board2Y = boardsStruct[myData.id-1].y;
  int board2Z = boardsStruct[myData.id-1].Chip_Temp;
    

    if( Flag == 0 )//Anded with flag to prevent repetition
    {
      //Debugging
      Serial.printf("x value: %d \n", board2X);
      Serial.printf("y value: %d \n", board2Y);
      Serial.printf("Z value: %d \n", board2Z);
      Serial.println();
      
      //Blynk.begin(auth, ssid, pass);
      Blynk.config(auth);
      Blynk.connectWiFi(ssid, pass);
      Blynk.connect();
      Blynk.run();

      led1.on();
      WLCD.clear();

      WLCD.print(0 , 0, board2X);
      WLCD.print(4 , 0, "Cm");
     // WLCD.print(6 , 0, board2Z);

      delay(300);
      led1.off();
      
      Flag++;
      WiFi.disconnect();
      WiFi.mode(WIFI_AP_STA);
      WiFi.disconnect();
      delay(300);
      while (esp_now_init() != 0) 
      {
        Serial.println("Error initializing ESP-NOW in main loop");
        return;
      }
  
    }
    
    
}

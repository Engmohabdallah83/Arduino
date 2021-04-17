/*
  This program is the interpretation routine of standard output protocol of TFmini-Plus product on Arduino.
  For details, refer to Product Specifications.
  For Arduino boards with only one serial port like UNO board, the function of software visual serial port is
  to be used.
*/
#include <Adafruit_GFX.h>      // include adafruit graphics library
#include <Adafruit_PCD8544.h>  // include adafruit PCD8544 (Nokia 5110) library

#include <SoftwareSerial.h> //header file of software serial port
SoftwareSerial Serial1(0 , 1) ; //define software serial port name as Serial1 and define pin2 as RX and pin3 as TX
/* For Arduinoboards with multiple serial ports like DUEboard, interpret above two pieces of code and
  directly use Serial1 serial port*/

// Nokia 5110 LCD module connections (CLK, DIN, D/C, CS, RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);

int dist; //actual distance measurements of LiDAR
int strength; //signal strength of LiDAR
float temprature;
int check; //save check value
int i;
int uart[9]; //save data measured by LiDAR
const int HEADER = 0x59; //frame header of data package
void setup() {

  Serial.begin(9600); //set bit rate of serial port connecting Arduino with computer
  Serial1.begin(115200); //set bit rate of serial port connecting LiDAR with Arduino

  delay(1000);  // wait 1 second
  
  // initialize the display
  display.begin();

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);
 
  display.clearDisplay();   // clear the screen and buffer
  display.display();

  //draw Line to divide display
  display.drawFastHLine(0, 23, display.width(), BLACK);

  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(6, 0);
  display.print("LEVEL IS:");
  display.setCursor(0, 28);
  display.print("SIGNAL PWR :");
  display.display();
}
void loop() {
  if (Serial1.available()) { //check if serial port has data input
    if (Serial1.read() == HEADER) { //assess data package frame header 0x59
      uart[0] = HEADER;
      if (Serial1.read() == HEADER) { //assess data package frame header 0x59
        uart[1] = HEADER;
        for (i = 2; i < 9; i++) { //save data in array
          uart[i] = Serial1.read();
        }
        check = uart[0] + uart[1] + uart[2] + uart[3] + uart[4] + uart[5] + uart[6] + uart[7];
        if (uart[8] == (check & 0xff)) { //verify the received data as per protocol
          dist = uart[2] + uart[3] * 256; //calculate distance value
          strength = uart[4] + uart[5] * 256; //calculate signal strength value
          temprature = uart[6] + uart[7] * 256; //calculate chip temprature
          temprature = temprature / 8 - 256;
          Serial.print("dist= ");
          Serial.print(dist); //output measure distance value of LiDAR
          Serial.print('\t');
          Serial.print(" strength= ");
          Serial.print(strength); //output signal strength value
          Serial.print("\tChip Temprature= ");
          Serial.print(temprature);
          Serial.println("ْ "); //output chip temperature of Lidar

          // print temperature
          display.fillRect(18, 12, display.width(), 8, WHITE);
          display.setCursor(18, 12);
          display.print(dist);
          //display.setCursor(44, 12);
          display.print(" Cm");

          // print humidity
          display.fillRect(18, 40, display.width(), 8, WHITE);
          display.setCursor(18, 40);
          display.print(strength);
          //display.setCursor(44, 40);
          display.print(" %");
          // now update the display
          display.display();

          delay(1500u);
        }
      }
    }
  }
}

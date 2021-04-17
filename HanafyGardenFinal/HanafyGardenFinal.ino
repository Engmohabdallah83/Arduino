/**************************************************************************

   ESP8266 NodeMCU real time clock with Nokia 5110 LCD and DS3231.
   This is a free software with NO WARRANTY.
   https://simple-circuit.com/

   Addition by Eng Mohamed Abdallah:
   ---------------------------------
   Added to this software Station Adjust
   Each station require Run Time period
   Number of Runing perday
   Days of week to run
   Exit Edit by Exit Button press
   Push all Configuration Data into EEPROM

 *************************************************************************/
#include <SPI.h>
#include <EEPROM.h>            // include library to read and write from flash memory
#include <stdlib.h>
#include <Wire.h>              // include Wire library (required for I2C devices)
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_PCD8544.h>  // include Adafruit PCD8544 (Nokia 5110) library
#include <RTClib.h>            // include Adafruit RTC library
/* ESP8266 wifi library */
#include <ESP8266WiFi.h>

/* Blynk Library*/
#include <BlynkSimpleEsp8266.h>
#include <WidgetRTC.h>
//#include <TimeLib.h>
/************************************************************************************
                                      TODO
 ************************************************************************************/


//TODO---> Put Blynk.run under constraints ---> if (WiFi.status()==WL_CONNECTED)
//                                                 { Blynk.run();
//                                                   Serial.println("Blynk timer run");}

//TODO---> Use blynk Timer e.g.:  timer.setInterval(250L, blynkAndButtonRead);
//                                timer.setInterval(2500L, runWaterLogic); // Watering methods
//                                timer.setInterval(15000L, rtcCheck); // RTC Module time update every 15 seconds

/**************************************************************************
                                    Macros
 **************************************************************************/
// define the number of bytes you want to access
#define EEPROM_SIZE 128U

#define I2CAddressESPWifi 6

#define TX_Expired    2000

#define NUMOFSTATION    5

#define RUN_PERIOD      6
#define WaterPerDay     7
#define RUNTIME1        8
#define RUNTIME2        9
#define RUNTIME3        10
#define RUNTIME4        11
#define RUNTIME5        12

#define DOW_Display   20
#define Lowercase_o   0x6F
#define _Asterix      0x2A

#define  MaxStation  4
#define  RUNPERDAY   5
#define  WeeKNofDays   7
#define Off_Day        30

#define Station1    1
#define Station2    2
#define Station3    3
#define Station4    4

#define First_Station     0
#define Second_Station    1
#define Third_Station     2
#define Fourth_Station    3

#define OPEN        HIGH
#define CLOSE       LOW
#define ReDisplay   2
#define Reset       0

#define EditB1PRESSED     digitalRead(button1)
#define IncrB2PRESSED     digitalRead(button2)
#define Exit              digitalRead(button3)

#define ThisIsDay_Period  ((ArrOfSt[St_Num].DOW_Run[Now.dayOfTheWeek()])==_Asterix )&&((Now.minute())< ArrOfSt[St_Num].StRunPeriod)

#define ThisIsRunTime    ((Now.hour())==(ArrOfSt[St_Num].RunTime[RunTnum]))

/*******************************************************************************************************************
  ( ( ArrOfSt[St_Num].DOW_Run[Now.dayOfTheWeek()] ) == _Asterix ) &&
           ((Now.hour()) == (ArrOfSt[St_Num].RunTime[RunTnum])) && ( (Now.minute()) < ArrOfSt[St_Num].StRunPeriod)
********************************************************************************************************************/
/****************************************************************************
                                  GLOBAL Varaiables
 ****************************************************************************/

const int button1 = D7;  // button B1 is connected to NodeMCU D7
const int button2 = D8;  // button B2 is connected to NodeMCU D8
const int button3 = D9;  // button B3 is connected to NodeMCU D9 EXIT


uint8_t Control = 0;

uint16_t TX_Wait = 0;     //Time waiting counter during I2C Tx in control station
uint8_t FeedBack = 0;     //FeedBack from ATtiny During Executing control station

byte i = 0;   //This variable is the step of edit taken in edit function

uint8_t NumOfStation = 4;

byte Flag = 0;

byte x;


typedef struct
{
  uint8_t StRunPeriod;
  uint8_t NumOfRunPerDay;
  uint8_t RunTime[5];
  char    DOW_Run[7];
} St_Data;

St_Data WidgetCfg;

St_Data ArrOfSt[4];

uint8_t  DOW_Cntr = 0;

uint8_t MenuStationSelect = 0 ;

uint8_t BlynkFlag = LOW;

uint8_t StDataAdressCtr = 0;

uint8_t NowStStatus[MaxStation];

uint8_t CurrentStStatus[MaxStation] = {CLOSE, CLOSE, CLOSE, CLOSE}; //deafault initial condition
uint8_t PrevStStatus[MaxStation] = {ReDisplay, ReDisplay, ReDisplay, ReDisplay}; //initially only

/*****************************************************/

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
const char auth[] = "m-B81niVl9VdiglxK-h8flNefcRd_B8-";

// Your WiFi credentials.
// Set password to "" for open networks.
const char* ssid = "Mohamed HUAWEI P9";
const char* pass = "asd@12345";

//char ssid[] = "MARTEN_EXT";
//char pass[] = "Medo96@Sondos92";
/*****************************************************/

/**********************************************************************************
                        Classes and Widget definition
 **********************************************************************************/

// Nokia 5110 LCD module connections (CLK, DIN, D/C, CS, RST)
Adafruit_PCD8544 display = Adafruit_PCD8544(D4, D3, D2, D1, D0);

// initialize RTC library
RTC_DS1307 rtc;
DateTime   Now;

WidgetLCD WLCD(V0);

WidgetLED led1(V1);
WidgetLED led2(V2);
WidgetLED led3(V3);
WidgetLED led4(V4); //register leds to virtual pin 1,2,3,4

WidgetRTC WRTC;

BlynkTimer timer;


/***********************************************************************************************************
            APPLICATION SECTIOM
                                APPLICATION SECTIOM
                                                    APPLICATION SECTIOM
                                                                        APPLICATION SECTIOM
 ***********************************************************************************************************/



/////////////////////////////////////////////////////////////////////////////////
//                                WLCD_LED_Start                               //
/////////////////////////////////////////////////////////////////////////////////
void WLCD_LED_Start(void)
{
  WLCD.clear(); //Use it to clear the LCD Widget
  WLCD.print(4, 0, "HELLO"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")
  WLCD.print(0, 1, "Console Hanafy"); // use: (position X: 0-15, position Y: 0-1, "Message you want to print")

  delay(4 * 1000UL);
  WLCD.clear();
  WLCD.print(0, 0, "Garden Watering");
  WLCD.print(0, 1, "System is Ready");
  delay(3 * 1000UL);
  WLCD.clear();
  WLCD.print(1, 0, "Auto Mode");
  WLCD.print(1, 1, "is the default");
  delay(3 * 1000UL);
  WLCD.clear();

  //All values between 0 and 255 will change LED brightness:
  led1.setValue(180); //set brightness of LED to 70%.
  led2.setValue(180); //set brightness of LED to 70%.
  led3.setValue(180); //set brightness of LED to 70%.
  led4.setValue(180); //set brightness of LED to 70%.

  led1.off();
  led2.off();
  led3.off();
  led4.off();
}
/************************** END of Basic WIDGET Display Function ************************/

/////////////////////////////////////////////////////////////////////////////////
//                                WLCD_Massages                                //
/////////////////////////////////////////////////////////////////////////////////
void WLCD_Massages( uint8_t u8LocMsg )
{
  switch (u8LocMsg)
  {
    case Cfg_Update :
      break;
    case St_Open :
      break;
    case St_Close :
      break;
  }

}
/************************** END WIDGET Display Function ************************/

/////////////////////////////////////////////////
//          Station 1 Manual Control           //
/////////////////////////////////////////////////

BLYNK_WRITE(V5)
{
  if (param.asInt())
  {
    //Put the station Status value into the new status
    //Call Control Function with Number of station and status required
    Control_Station(First_Station , OPEN);
  }
  else
  {
    //Call Control Function with Number of station and status required
    //Put the station Status value into the new status
    Control_Station(First_Station , CLOSE);

  }
}

/////////////////////////////////////////////////
//          Station 2 Manual Control           //
/////////////////////////////////////////////////

BLYNK_WRITE(V6)
{
  if (param.asInt())
  {
    //Put the station Status value into the new status
    //Call Control Function with Number of station and status required

    Control_Station(Second_Station , OPEN);
  }
  else
  {
    //Call Control Function with Number of station and status required
    //Put the station Status value into the new status
    Control_Station(Second_Station , CLOSE);
  }
}

/////////////////////////////////////////////////
//          Station 3 Manual Control           //
/////////////////////////////////////////////////

BLYNK_WRITE(V7)
{
  if (param.asInt())
  {
    //Put the station Status value into the new status
    //Call Control Function with Number of station and status required

    Control_Station(Third_Station , OPEN);
  }
  else
  {
    //Call Control Function with Number of station and status required
    //Put the station Status value into the new status

    Control_Station(Third_Station , CLOSE);
  }
}

/////////////////////////////////////////////////
//          Station 4 Manual Control           //
/////////////////////////////////////////////////

BLYNK_WRITE(V8)
{
  if (param.asInt())
  {
    //Put the station Status value into the new status
    //Call Control Function with Number of station and status required

    Control_Station(Fourth_Station , OPEN);
  }
  else
  {
    //Call Control Function with Number of station and status required
    //Put the station Status value into the new status
    Control_Station(Fourth_Station , CLOSE);
  }
}

///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////


BLYNK_WRITE(V9)
{
  MenuStationSelect = ( ( param.asInt() ) - 1 );
}
///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

BLYNK_WRITE(V10)
{
  WidgetCfg.StRunPeriod = param.asInt();
}

///////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////

BLYNK_WRITE(V11)
{
  WidgetCfg.NumOfRunPerDay = param.asInt();
}

/////////////////////////////////////////////////////////////////////
//             Station Run Time1 and Weeks of Running              //
/////////////////////////////////////////////////////////////////////
BLYNK_WRITE(V12)
{
  TimeInputParam t(param);

  // Process start time

  if (t.hasStartTime())
  {
    WidgetCfg.RunTime[0] = t.getStartHour();
  }

  // Process weekdays (1. Mon, 2. Tue, 3. Wed, ...)

  for (uint8_t i = 1; i <= 7; i++)
  {
    if (t.isWeekdaySelected(i))
    {
      //Serial.println(String("Day ") + i + " is selected");
      WidgetCfg.DOW_Run[i - 1] = '*';
    }
  }

  //Serial.println();
}

/////////////////////////////////////////////////////////////////////
//                       RUN Time 2 Function                       //
/////////////////////////////////////////////////////////////////////

BLYNK_WRITE(V13)
{
  TimeInputParam t(param);

  // Process start time

  if (t.hasStartTime())
  {
    WidgetCfg.RunTime[1] = t.getStartHour();
  }
}

/////////////////////////////////////////////////////////////////////
//                       RUN Time 3 Function                       //
/////////////////////////////////////////////////////////////////////


BLYNK_WRITE(V14)
{
  TimeInputParam t(param);

  // Process start time

  if (t.hasStartTime())
  {
    WidgetCfg.RunTime[2] = t.getStartHour();
  }
}

/////////////////////////////////////////////////////////////////////
//                       RUN Time 4 Function                       //
/////////////////////////////////////////////////////////////////////


BLYNK_WRITE(V15)
{
  TimeInputParam t(param);

  // Process start time

  if (t.hasStartTime())
  {
    WidgetCfg.RunTime[3] = t.getStartHour();
  }
}

/////////////////////////////////////////////////////////////////////
//                       RUN Time 5 Function                       //
/////////////////////////////////////////////////////////////////////


BLYNK_WRITE(V16)
{
  TimeInputParam t(param);

  // Process start time

  if (t.hasStartTime())
  {
    WidgetCfg.RunTime[4] = t.getStartHour();
  }
}


///////////////////////////////////////////////////////////////////////////
//                        Widget Clock Synch                             //
///////////////////////////////////////////////////////////////////////////

// Digital clock display of the time
void clockDisplay()
{
  // You can call hour(), minute(), ... at any time
  // Please see Time library examples for details

  WLCD.clear();
  WLCD.print(0, 0, hour());
  WLCD.print(3, 0, ":");
  WLCD.print(5, 0, minute());

}

////////////////////////////////////////////////////////////////////////
BLYNK_CONNECTED()
{
  // Synchronize time on connection
  WRTC.begin();
}

///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BLYNK_WRITE(V18)
{
  //if Saving Config Button Pressed Call saving config Function
  NumOfStation = param.asInt();

}


///////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BLYNK_WRITE(V17)
{
  //if Saving Config Button Pressed Call saving config Function
  if (param.asInt())
  {
    SavingConfig(MenuStationSelect);
    //Save the last Config to EEPROM
    SaveCfgToEEPROM();
  }
}



/////////////////////////////////////////////////////////////////////
//              Saving Configuration Data Function                 //
/////////////////////////////////////////////////////////////////////

void SavingConfig(uint8_t Station)
{
  ArrOfSt[Station].StRunPeriod = WidgetCfg.StRunPeriod;
  ArrOfSt[Station].NumOfRunPerDay = WidgetCfg.NumOfRunPerDay;
  for (uint8_t R_Times = 0; R_Times < RUNPERDAY; R_Times++)
  {
    ArrOfSt[Station].RunTime[R_Times] = WidgetCfg.RunTime[R_Times];
  }

  for (uint8_t W_day = 0; W_day < WeeKNofDays; W_day++)
  {
    ArrOfSt[Station].DOW_Run[W_day] = WidgetCfg.DOW_Run[W_day];
  }

  //sprintf( ArrOfSt[0].DOW_Run , "%c" , WidgetCfg.DOW_Run);

  WLCD.clear();
  WLCD.print(0, 0, ArrOfSt[Station].StRunPeriod);
  WLCD.print(0, 1, ArrOfSt[Station].NumOfRunPerDay);

  delay(4000);
  WLCD.clear();

  WLCD.print(0 , 0, ArrOfSt[Station].RunTime[0]);
  WLCD.print(3 , 0, ArrOfSt[Station].RunTime[1]);
  WLCD.print(6 , 0, ArrOfSt[Station].RunTime[2]);
  WLCD.print(9 , 0, ArrOfSt[Station].RunTime[3]);
  WLCD.print(12, 0, ArrOfSt[Station].RunTime[4]);

  delay(4000);
  WLCD.clear();

  uint8_t W_days , X_axis = 0;

  do
  {
    WLCD.print(X_axis , 1, ArrOfSt[Station].DOW_Run[W_days]);
    W_days++;
    X_axis += 2;
  } while (W_days < 7);

  delay(4000);
  clockDisplay();

}


/***********************************************************************************************************
         HARDWARE SECTIOM
                              HARDWARE SECTIOM
                                                     HARDWARE SECTIOM
                                                                            HARDWARE SECTIOM
 ***********************************************************************************************************/

/********************************************************************
            a small function for button1 (B1) debounce
 *******************************************************************/

bool debounce ()
{
  byte count = 0;
  for (byte i = 0; i < 5; i++)
  {
    if (EditB1PRESSED)
      count++;
    delay(10);
  }

  if (count > 2)  return 1;
  else           return 0;
}
/*************************** Debounce (B1) END *************************/

/***************************************************
           Basic Display Function
 ***************************************************/

void Basic_Display()
{
  display.clearDisplay();   // clear the screen and buffer
  display.setCursor(0, 16);
  display.print("Time:");
  display.display();

  for (uint8_t StCount = 0; StCount < MaxStation; StCount++)
  {
    //Close all station to return to default status
    CloseStation(StCount);
    CurrentStStatus[StCount] = PrevStStatus[StCount] = CLOSE; //Reset Run Times flag in order to redisplay
  }

  delay(100);
}

/*****************************************************************************
                            Display Configuration
 ****************************************************************************/
void Config_Display(uint8_t u8Config_Step)
{
  switch (u8Config_Step)
  {
    case NUMOFSTATION:
      /* Next will Take user to Next page
        therefore a new page will be displayed with New station
        Number of Station displayed*/
      display.clearDisplay();   // clear the screen and buffer
      display.setCursor(0, 0);
      display.print("Enter Number");
      display.println("of Station:");
      display.println("Max 4 St.:");
      display.setCursor(40, 24);
      display.print("Station");
      display.display();
      break;
    case RUN_PERIOD:
      /* Next will Take user to Next page
        therefore a new page will be displayed with New station
        Run Period displayed*/
      display.clearDisplay();   // clear the screen and buffer
      display.setCursor(0, 0);
      display.print("Enter Station ");
      display.printf("%u", x + 1);
      display.println(" Run Period:");
      display.println("Max 59 min:");
      display.setCursor(40, 24);
      display.print("min");
      display.display();
      break;
    case WaterPerDay:
      /* Next will Take user to Next page
        therefore a new page will be displayed with New station
        Run Period displayed*/
      display.clearDisplay();   // clear the screen and buffer
      display.setCursor(0, 0);
      display.print("Watering Per");
      display.println("Day:");
      display.println("Max 5 Times.");
      display.display();
      break;
    case RUNTIME1:
      /* Next will Take user to Next page
        therefore a new page will be displayed with New station
        Run Period displayed*/
      display.clearDisplay();   // clear the screen and buffer
      display.setCursor(0, 0);
      display.print("Enter Start");
      display.println("of Time1:");
      display.display();
      break;
    case RUNTIME2:
      /* Next will Take user to Next page
        therefore a new page will be displayed with New station
        Run Period displayed*/
      display.clearDisplay();   // clear the screen and buffer
      display.setCursor(0, 0);
      display.print("Enter Start");
      display.println("of Time2:");
      display.display();
      break;
    case RUNTIME3:
      /* Next will Take user to Next page
        therefore a new page will be displayed with New station
        Run Period displayed*/
      display.clearDisplay();   // clear the screen and buffer
      display.setCursor(0, 0);
      display.print("Enter Start");
      display.println("of Time3:");
      display.display();
      break;
    case RUNTIME4:
      /* Next will Take user to Next page
        therefore a new page will be displayed with New station
        Run Period displayed*/
      display.clearDisplay();   // clear the screen and buffer
      display.setCursor(0, 0);
      display.print("Enter Start");
      display.println("of Time4:");
      display.display();
      break;
    case RUNTIME5:
      /* Next will Take user to Next page
        therefore a new page will be displayed with New station
        Run Period displayed*/
      display.clearDisplay();   // clear the screen and buffer
      display.setCursor(0, 0);
      display.print("Enter Start");
      display.println("of Time5:");
      display.display();
      break;

    case DOW_Display :
      display.clearDisplay();   // clear the screen and buffer
      display.setCursor(0, 0);
      display.print("o Sun, o Mon");
      display.setCursor(0, 8);
      display.print("o Tue, o Wed");
      display.setCursor(0, 16);
      display.print("o Thr, o Fri");
      display.setCursor(0, 24);
      display.print("o Sat");
      display.display();
      break;

  }
}
/************************* END of Configuration DISPLAY *************************/


//////////////////////////////////////////////////////////////////
//              Saving Configuration Data in EEPROM             //
//////////////////////////////////////////////////////////////////

void SaveCfgToEEPROM(void)
{
  StDataAdressCtr = 0;

  EEPROM.write( StDataAdressCtr , NumOfStation );
  StDataAdressCtr++;
  //Loop on Number of station Activated
  for (uint8_t St_Ctr = 0 ; St_Ctr < NumOfStation ; St_Ctr++ )
  {
    EEPROM.write( StDataAdressCtr , ArrOfSt[St_Ctr].StRunPeriod );
    StDataAdressCtr++;
    EEPROM.write( StDataAdressCtr , ArrOfSt[St_Ctr].NumOfRunPerDay);
    StDataAdressCtr++;
    for (uint8_t RT_Ctr = 0 ; RT_Ctr < RUNPERDAY ; RT_Ctr++ )
    {
      EEPROM.write( StDataAdressCtr , ArrOfSt[St_Ctr].RunTime[RT_Ctr]);
      StDataAdressCtr++;
    }
    for (uint8_t DOW_Ctr = 0 ; DOW_Ctr < WeeKNofDays ; DOW_Ctr++ )
    {
      EEPROM.write( StDataAdressCtr , ArrOfSt[St_Ctr].DOW_Run[DOW_Ctr]);
      StDataAdressCtr++;
    }//DOW Ctr For END
  }//Stations Ctr For END
  //Save Data t EEPROM
  EEPROM.commit();
  //Reset Address Ctr
  //StDataAdressCtr = 0;
}//SaveCfgToEEPROM Fun END

////////////////////////////////////////////////////////////////////
//                  Read Configuration from EEPROM                //
////////////////////////////////////////////////////////////////////
void GetCfgFromEEPROM(void)
{
  StDataAdressCtr = 0;

  NumOfStation  = EEPROM.read(StDataAdressCtr);
  StDataAdressCtr++;
  //Loop on Number of station Activated
  for (uint8_t St_Ctr = 0 ; St_Ctr < NumOfStation ; St_Ctr++ )
  {
    ArrOfSt[St_Ctr].StRunPeriod  = EEPROM.read(StDataAdressCtr);
    StDataAdressCtr++;
    ArrOfSt[St_Ctr].NumOfRunPerDay  = EEPROM.read( StDataAdressCtr );
    StDataAdressCtr++;
    for (uint8_t RT_Ctr = 0 ; RT_Ctr < RUNPERDAY ; RT_Ctr++ )
    {
      ArrOfSt[St_Ctr].RunTime[RT_Ctr] = EEPROM.read( StDataAdressCtr );
      StDataAdressCtr++;
    }
    for (uint8_t DOW_Ctr = 0 ; DOW_Ctr < WeeKNofDays ; DOW_Ctr++ )
    {
      ArrOfSt[St_Ctr].DOW_Run[DOW_Ctr] = EEPROM.read( StDataAdressCtr );
      StDataAdressCtr++;
    }//DOW Ctr For END
  }
  //Reset Address
  //StDataAdressCtr = 0;

}

///////////////////////////////////////////////////////////////////////
//                  Read Configuration from EEPROM END               //
///////////////////////////////////////////////////////////////////////

/**********************************************************************
                          Display Date Time
 **********************************************************************/
void RTC_display()
{
  char dow_matrix[7][10] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
  byte x_pos[7] = {24, 24, 21, 15, 18, 24, 18};
  static byte previous_dow = 8;

  // print day of the week
  if ( previous_dow != Now.dayOfTheWeek() )
  {
    previous_dow = Now.dayOfTheWeek();
    display.fillRect(15, 0, 54, 8, WHITE);     // draw rectangle (erase day from the display)
  }
  display.setCursor(x_pos[previous_dow], 0);
  display.print( dow_matrix[Now.dayOfTheWeek()] );

  // print date
  display.setCursor(12, 8);
  display.printf( "%02u-%02u-%04u", Now.day(), Now.month(), Now.year() );
  // print time
  display.setCursor(28, 16);
  display.printf( "%02u:%02u:%02u", Now.hour(), Now.minute(), Now.second() );
  // now update the screen
  display.display();
}

/********************************** END of RTC DISPLAY *************************/

/*******************************************************************************
                             EDIT Day of Week Function
 *******************************************************************************/
byte DOW_edit(byte Parameter)
{
  static byte X_dowPos[2] = {42, 0} , Y_dowPos[4] = {0, 8, 16, 24};
  static byte x_axis = 1, y_axis = 0 , Display_value = 0 ;

  if (DOW_Cntr == 0)
  {

    x_axis = 1;
    y_axis = 0;
  }

  while ( debounce() );  // call debounce function (wait for B1 to be released)
  while (!(Exit))
  { //while exit button is not pressed
    while ( IncrB2PRESSED )
    { // while B2 is pressed
      Display_value ^= 1; //convert between 0 , 1 to give values "*" or "o"
      while ( !(IncrB2PRESSED) );        // wait until button 2 (Increment) Released
      if (Display_value == 0)   //every press it convert if 0 --> "o"
        Parameter = Lowercase_o;   //Display to user Lowercase o mean not selected
      else
        Parameter = _Asterix;   //Display to user Asterix mean selected

      display.setCursor(X_dowPos[x_axis], Y_dowPos[y_axis]);
      display.printf("%c", Parameter);
      display.display();  // update the screen
    }

    //flashing

    display.fillRect(X_dowPos[x_axis], Y_dowPos[y_axis], 11, 8, WHITE);
    display.display();  // update the screen
    unsigned long previous_m = millis();
    while ( (millis() - previous_m < 280) && !EditB1PRESSED && !IncrB2PRESSED) ;
    display.setCursor(X_dowPos[x_axis], Y_dowPos[y_axis]);
    display.printf("%c", Parameter);
    display.display();  // update the screen
    previous_m = millis();
    while ( (millis() - previous_m < 280) && !EditB1PRESSED && !IncrB2PRESSED) ;

    //if Button B1 is pressed (Transfer button)
    if (EditB1PRESSED)
    {
      x_axis = (x_axis + 1) % 2;        //Transfer to the next Lateral place the next day
      y_axis = (y_axis + x_axis) % 4 ;  //Transfer to the next line vertical axis
      DOW_Cntr = (DOW_Cntr + 1) % 7;

      return Parameter;
    }
  }
  return Parameter;
}
//////////////////////////////////////////////////////////////////
//             Days of week Parameters Display                  //
//////////////////////////////////////////////////////////////////
void DOW_valDisplay(char  Parameter)
{
  static byte X_dowPos[2] = {42, 0} , Y_dowPos[4] = {0, 8, 16, 24};
  static byte x_axis = 1, y_axis = 0 ;

  if (DOW_Cntr == 0)
  {
    x_axis = 1;
    y_axis = 0;
  }

  display.setCursor(X_dowPos[x_axis], Y_dowPos[y_axis]);
  display.printf("%c", Parameter);
  display.display();  // update the screen

  x_axis = (x_axis + 1) % 2;        //Transfer to the next Lateral place the next day
  y_axis = (y_axis + x_axis) % 4 ;  //Transfer to the next line vertical axis
  DOW_Cntr = (DOW_Cntr + 1) % 7;
}



/********************************** END of DOW EDIT Fun *************************/


/*******************************************************************************

   EDIT Parameter Function

 *******************************************************************************/
byte edit(byte parameter)
{
  static byte y_pos,
         x_pos[13] = {12, 30, 60, 28, 45, 12, 12, 12, 12, 12, 12, 12, 12};
  if (i < 3)
    y_pos = 8;
  else if (i < 5)
    y_pos = 16;
  else
  {
    Config_Display(i);
    y_pos = 24;
  }

  while ( debounce() );  // call debounce function (wait for B1 to be released)

  while (!(Exit) )
  { //while Exit button did not pressed increment and continue
    while (IncrB2PRESSED) { // while B2 is pressed
      parameter++;
      if (i == 0 && parameter > 31)   // if day > 31 ==> day = 1
        parameter = 1;
      if (i == 1 && parameter > 12)   // if month > 12 ==> month = 1
        parameter = 1;
      if (i == 2 && parameter > 99)   // if year > 99 ==> year = 0
        parameter = 0;
      if (i == 3 && parameter > 23)   // if hours > 23 ==> hours = 0
        parameter = 0;
      if (i == 4 && parameter > 59)   // if minutes > 59 ==> minutes = 0
        parameter = 0;
      if (i == 5 && parameter > 4)   // if Number of station > 4 ==> Station = 0
        parameter = 1;
      if (i == 6 && parameter > 59)   // if Period > 59 ==> Period = 0
        parameter = 0;
      if (i == 7 && parameter > 5)   // if NumOfRunPerDay > 5 ==> NumOfRunPerDay = 0
        parameter = 0;
      if (i > 7 && parameter > 23)   // if Run Time > 23 ==> Run Time = 0
        parameter = 0;



      display.setCursor(x_pos[i], y_pos);
      display.printf("%02u", parameter);
      display.display();  // update the screen
      delay(230);         // wait 200ms
    }

    display.fillRect(x_pos[i], y_pos, 11, 8, WHITE);
    display.display();  // update the screen
    unsigned long previous_m = millis();
    while ( (millis() - previous_m < 280) && !EditB1PRESSED && !IncrB2PRESSED) ;
    display.setCursor(x_pos[i], y_pos);
    display.printf("%02u", parameter);
    display.display();  // update the screen
    previous_m = millis();
    while ( (millis() - previous_m < 280) && !EditB1PRESSED && !IncrB2PRESSED) ;

    if (EditB1PRESSED)
    { // if button B1 is pressed
      i = (i + 1) % 13;    // increment 'i' for the next parameter
      return parameter;   // return parameter value and exit
    }

  }//while(!Exit) END
  return parameter;
}
/*********************************** END OF EDIT FUNCTION ***********************************/


/*******************************************************************************************
                  Open_Station
********************************************************************************************
  INPUT: Station Number
*******************************************************************************************/
void OpenStation(uint8_t StationNum)
{
  TX_Wait = millis();

  do {
    Wire.beginTransmission(I2CAddressESPWifi);
    Wire.write(StationNum);
    Wire.endTransmission();
    delay(200);
    Wire.requestFrom(I2CAddressESPWifi, 1);
    while (Wire.available())
    {
      FeedBack = Wire.read();
    }
  } while ( ( ((millis() ) - TX_Wait) < TX_Expired ) && !(FeedBack) );

}
/*******************************************************************
                  Close station
 *******************************************************************
  Input: station Number
 ***************************************************************/
void CloseStation(uint8_t StationNum)
{
  StationNum = StationNum + 4 ;

  TX_Wait = millis();

  do {
    Wire.beginTransmission(I2CAddressESPWifi);
    Wire.write(StationNum);
    Wire.endTransmission();
    delay(200);
    Wire.requestFrom(I2CAddressESPWifi, 1);
    while (Wire.available())
    {
      FeedBack = Wire.read();
    }
  } while ( ( ((millis() ) - TX_Wait) < TX_Expired ) && !(FeedBack) );
}

/*******************************************************************************************
                  Control_Station
********************************************************************************************
  INPUT: Station Number

*******************************************************************************************/
uint8_t Control_Station(uint8_t Station, uint8_t Status)
{
  switch (Station)
  {
    case First_Station:

      if (Status == OPEN)
      {
        OpenStation(Station1); //open Desired station valve Function
        if (FeedBack == 1)
        {
          display.setCursor(0, 24);
          display.print("S.1" );

          display.setCursor(0, 40);
          display.printf("%d", FeedBack);

          display.display();  // update the screen

          if ( Blynk.connected() )          //Ceck connection to Blynk Server
          {
            led1.on();
          }

        }
      }
      else if (Status == CLOSE)
      {
        CloseStation(Station1); //Close Desired station valve Function
        if (FeedBack == 1)
        {
          display.fillRect(0, 24, 42, 8, WHITE);     // draw rectangle (erase day from the display)

          display.setCursor(10, 40);
          display.printf("%d", FeedBack);

          display.display();  // update the screen

          if ( Blynk.connected() )          //Ceck connection to Blynk Server
          {
            led1.off();
          }
        }
      }

      break;

    case Second_Station:

      if (Status == OPEN)
      {
        OpenStation(Station2); //open Desired station valve Function
        if (FeedBack == 1)
        {
          display.setCursor(43, 24);
          display.print("S.2 ");

          display.setCursor(20, 40);
          display.printf("%d", FeedBack);

          display.display();  // update the screen

          if ( Blynk.connected() )          //Ceck connection to Blynk Server
          {
            led2.on();
          }
        }
      }
      else if (Status == CLOSE)
      {
        CloseStation(Station2); //Close Desired station valve Function
        if (FeedBack == 1)
        {
          display.fillRect(43, 24, 42, 8, WHITE);     // draw rectangle (erase day from the display)

          display.setCursor(30, 40);
          display.printf("%d", FeedBack);

          display.display();  // update the screen

          if ( Blynk.connected() )          //Ceck connection to Blynk Server
          {
            led2.off();
          }
        }
      }
      break;

    case Third_Station:

      if (Status == OPEN)
      {
        OpenStation(Station3); //open Desired station valve MACRO Function
        if (FeedBack == 1)
        {
          display.setCursor(0, 32 );
          display.print("S.3" );

          display.setCursor(40, 40);
          display.printf("%d", FeedBack);

          display.display();  // update the screen
          if ( Blynk.connected() )          //Ceck connection to Blynk Server
          {
            led3.on();
          }
        }
      }
      else if (Status == CLOSE)
      {
        CloseStation(Station3); //Close Desired station valve MACRO Function
        if (FeedBack == 1)
        {
          display.fillRect(0, 32, 42, 8, WHITE);     // draw rectangle (erase day from the display)

          display.setCursor(50, 40);
          display.printf("%d", FeedBack);

          display.display();  // update the screen

          if ( Blynk.connected() )          //Ceck connection to Blynk Server
          {
            led3.off();
          }
        }
      }
      break;

    case Fourth_Station:

      if (Status == OPEN)
      {
        OpenStation(Station4); //open Desired station valve MACRO Function
        if (FeedBack == 1)
        {
          display.setCursor(43, 32);
          display.print("S.4");

          display.setCursor(60, 40);
          display.printf("%d", FeedBack);

          display.display();  // update the screen
          if ( Blynk.connected() )          //Ceck connection to Blynk Server
          {
            led4.on();
          }
        }
      }
      else if (Status == CLOSE)
      {
        CloseStation(Station4); //Close Desired station valve MACRO Function
        if (FeedBack == 1)
        {
          display.fillRect(43, 32, 42, 8, WHITE);     // draw rectangle (erase day from the display)

          display.setCursor(70, 40);
          display.printf("%d", FeedBack);

          display.display();  // update the screen

          if ( Blynk.connected() )          //Ceck connection to Blynk Server
          {
            led4.off();
          }
        }
      }
      break;
  }

  return FeedBack;
}//Control Station Fun END

/*******************************************************************************************
   Setup GPIO of Edit buttons
   Initialize Nokia5110 LCD
   Adjust Display Contrast
   Adjust Font Size
   Adjust Default Color
   Adjust Time word place by setcursor
   Print Time text
   Call Display to display the previous data

   initialize I2C and specify the SCl= D5 ,SDA = D6 Pins with default clock
   initialize RTC Module

 ************************************************************************************/
void setup(void)
{
  pinMode(button1, INPUT);  //Navigate
  pinMode(button2, INPUT);  //Increment
  pinMode(button3, INPUT);  //Exit

//Nokia 5110 LCD initialization
  // initialize the display
  display.begin();
  // you can change the contrast around to adapt the display
  display.setContrast(50);// for the best viewing!

  display.clearDisplay();   // clear the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  
  display.setCursor(0, 16);
  display.print("Time:");
  display.display();

//I2C initialization for Both RTC and Arduino nano
  Wire.begin(D6, D5);  // set I2C pins [SDA = D6, SCL = D5], default clock is 100kHz
  rtc.begin();         // initialize RTC chip

  WiFi.begin((char*)ssid, (char*)pass); // Connection to a wifi network
  Blynk.config(auth); // Connection to Blynk servers, 5 seconds timeout
  
  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);

  //Read Last Configuration from EEPROM
  GetCfgFromEEPROM();

  // Debug console
  //Serial.begin(9600);

  delay(2000u);//delay for connection Trial

  //continue trying to connect to Blynk server. Returns true when connected,
  //false if timeout have been reached. Default timeout is 30 seconds.
  while ( !(Blynk.connect(8000U)) );

  if ( Blynk.connected() )
  {
    setSyncInterval(10 * 60); // Sync interval in seconds (10 minutes)
    WLCD_LED_Start();
    //timer.setInterval(100UL, BlynkRun);
  }
}
/********************************* END OF SETUP *************************************/

/************************************************************************************
                                    Main Function
 ************************************************************************************




 ************************************************************************************/

void loop()
{

  //////////////////////////////////////////////////////////////////////////////////
  //             Configuration Edit Part                                          //
  //////////////////////////////////////////////////////////////////////////////////
  if ( !(Exit) )
  {
    if ( EditB1PRESSED ) // if B1 is pressed (EDIT)
    {
      if ( debounce() )            // call debounce function (make sure B1 is pressed)
      {
        while ( debounce() ); // call debounce function (wait for B1 to be released)

        byte day    = edit( Now.day() );          // edit date
        byte month  = edit( Now.month() );        // edit month
        byte year   = edit( Now.year() - 2000 );  // edit year
        byte hour   = edit( Now.hour() );         // edit hours
        byte minute = edit( Now.minute() );       // edit minutes

        // write time & date data to the RTC chip
        rtc.adjust(DateTime(2000 + year, month, day, hour, minute, 0));

        //Prompt user to enter Number of station
        NumOfStation = edit( NumOfStation );

        //Loop on station data
        for (uint8_t stCntr = 0 ; stCntr < NumOfStation ; stCntr++)
        {
          x = stCntr;

          if (Exit)
            break;

          ArrOfSt[stCntr].StRunPeriod = edit(ArrOfSt[stCntr].StRunPeriod);
          ArrOfSt[stCntr].NumOfRunPerDay = edit(ArrOfSt[stCntr].NumOfRunPerDay);
          //Loop on period start times
          for (uint8_t R_Times = 0 ; R_Times < ArrOfSt[stCntr].NumOfRunPerDay ; R_Times++)
          {
            if (Exit)
              break;

            ArrOfSt[stCntr].RunTime[R_Times] = edit(ArrOfSt[stCntr].RunTime[R_Times]);

          }//Run Times Loop

          i = 6;  //back to cursor station period step location


          ////////////////// obtain watering days of week ////////////////////////////
          /*Call the next page which is DOW_To_Run Select*/
          Config_Display(DOW_Display);

          /* Display firstly values of Days of weeks selections*/
          for (uint8_t dayNum = 0 ; dayNum < WeeKNofDays ; dayNum++)
          {

            DOW_valDisplay(ArrOfSt[stCntr].DOW_Run[dayNum]);

            if (Exit)   //any time exit button pressed break from the loop
            {
              DOW_Cntr = 0;
              break;
            }    // break from the loop

          }
          for (uint8_t dayNum = 0 ; dayNum < WeeKNofDays ; dayNum++)
          {
            ArrOfSt[stCntr].DOW_Run[dayNum] = DOW_edit(ArrOfSt[stCntr].DOW_Run[dayNum]);

            if (Exit)
            {
              DOW_Cntr = 0; //Reset Counter
              break;
            }
          }//Day Num Loop
        }//End For loop NumOfStation

        while (debounce()); // call debounce function (wait for button B1 to be released)

        i = 0;      // Reset i to return display cursor to position {0,0}

        Basic_Display();
        SaveCfgToEEPROM();

      }//End of EDIT

    }//Edit B1 Pressed
  }//
  //////////////////////////////////////// END of Edit /////////////////////////////////////////////////////

  /////////////////////////////////////// Display Time /////////////////////////////////////////////////////
  Now = rtc.now();  // read current time and date from the RTC chip

  RTC_display();   // display time & calendar
  delay(100);      // wait 100 ms

  /*****************************************
                Blynk Widget Part
   ****************************************/
  if ( WiFi.status() ==  WL_CONNECTED) //Check WiFi
  {
    Blynk.run();
  }

  /********************************************************************************************
                      Apply Configuration Part
  *********************************************************************************************
      Read Days of week to Run in each station
      check how many station
      check watering days of each station
      in each day check Run Times per day
      open each station according to Time of Run
  *********************************************************************************************/

  /* Loop on Number of Station */
  for ( uint8_t St_Num = 0 ; St_Num < NumOfStation ; St_Num++)
  {
    CurrentStStatus[St_Num] = CLOSE;

    if (ThisIsDay_Period)
    { /* Loop on Number of run per day to this station */
      for (uint8_t RunTnum = 0 ; RunTnum < ArrOfSt[St_Num].NumOfRunPerDay ; RunTnum++)
      {
        /* if any Run Time reach open time therefore this station will be opened */
        if (ThisIsRunTime)
        {
          CurrentStStatus[St_Num] = OPEN;
        }
      }
    }

    if ( CurrentStStatus[St_Num] != PrevStStatus[St_Num] )
    {
      if (CurrentStStatus[St_Num] == OPEN)
      {
        FeedBack = Control_Station(St_Num, OPEN);  //Open Current station
        if (FeedBack == 1)
        {
          PrevStStatus[St_Num] = CurrentStStatus[St_Num];
          FeedBack = Reset;
        }
      }
      else if (CurrentStStatus[St_Num] == CLOSE)
      {
        FeedBack = Control_Station(St_Num, CLOSE); //Close Current Station
        if (FeedBack == 1)
        {
          PrevStStatus[St_Num] = CurrentStStatus[St_Num];
          FeedBack = Reset;
        }//FeedBack if
      }
    }
  }//For loop on station
}// end of code.

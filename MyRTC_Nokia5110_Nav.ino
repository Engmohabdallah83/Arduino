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

// include library to read and write from flash memory
#include <SPI.h>               // include SPI library
#include <EEPROM.h>
#include <stdlib.h>
#include <Wire.h>              // include Wire library (required for I2C devices)
#include <Adafruit_GFX.h>      // include Adafruit graphics library
#include <Adafruit_PCD8544.h>  // include Adafruit PCD8544 (Nokia 5110) library
#include <RTClib.h>            // include Adafruit RTC library


/**************************************************************************
                                    Macros
 **************************************************************************/

// define the number of bytes you want to access
//#define EEPROM_SIZE 128U

//#define I2CAddressESPWifi 6

#define  MaxStation  4
#define  RUNPERDAY   5

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

#define  WeeKNofDays   7

#define First_Station     0
#define Second_Station    1
#define Third_Station     2
#define Fourth_Station    3

#define OPEN        HIGH
#define CLOSE       LOW

#define OpenStation1()    Wire.beginTransmission(I2CAddressESPWifi);\
                          Wire.write(1);\
                          Wire.endTransmission()

#define OpenStation2()    Wire.beginTransmission(I2CAddressESPWifi);\
                          Wire.write(2);\
                          Wire.endTransmission()

#define OpenStation3()    Wire.beginTransmission(I2CAddressESPWifi);\
                          Wire.write(3);\
                          Wire.endTransmission()

#define OpenStation4()    Wire.beginTransmission(I2CAddressESPWifi);\
                          Wire.write(4);\
                          Wire.endTransmission()

#define CloseStation1()   Wire.beginTransmission(I2CAddressESPWifi);\
                          Wire.write(11);\
                          Wire.endTransmission()

#define CloseStation2()   Wire.beginTransmission(I2CAddressESPWifi);\
                          Wire.write(22);\
                          Wire.endTransmission()

#define CloseStation3()   Wire.beginTransmission(I2CAddressESPWifi);\
                          Wire.write(33);\
                          Wire.endTransmission()

#define CloseStation4()   Wire.beginTransmission(I2CAddressESPWifi);\
                          Wire.write(44);\
                          Wire.endTransmission()


#define Exit            digitalRead(button3)

/**************************************************************************
                              Global Variables
 **************************************************************************/
  // Nokia 5110 LCD module connections (CLK, DIN, D/C, CS, RST)
  Adafruit_PCD8544 display = Adafruit_PCD8544(D4, D3, D2, D1, D0);
  
  // initialize RTC library
  RTC_DS1307 rtc;
  DateTime   now;

/************************************************************************
 ************************************************************************/

const int button1 = D7;  // button B1 is connected to NodeMCU D7
const int button2 = D8;  // button B2 is connected to NodeMCU D8
const int button3 = D9;  // button B3 is connected to NodeMCU D9 EXIT


byte i = 0;   //This variable is the step of edit taken in edit function

uint8_t NumOfStation = 4;

byte Flag = 0;

byte x;

typedef struct
{
  uint8_t StRunPeriod;
  uint8_t NumOfRunPerDay;
  uint8_t RunTime1;
  uint8_t RunTime2;
  uint8_t RunTime3;
  uint8_t RunTime4;
  uint8_t RunTime5;
  char    DOW_Run[7];
} St_Data;

St_Data St1, St2, St3, St4;

St_Data ArrOfSt[] = {St1, St2, St3, St4};

uint8_t RunTimeFlag[MaxStation][RUNPERDAY];

uint8_t  DOW_Cntr = 0;

uint8_t StDataAdressCtr = 0;

/*********************************** End of Global Variables *******************************/

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

  // initialize the display
  display.begin();

  // you can change the contrast around to adapt the display
  // for the best viewing!
  display.setContrast(50);

  display.clearDisplay();   // clear the screen and buffer
  display.setTextSize(1);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0, 16);
  display.print("Time:");
  display.display();

  Wire.begin(D6, D5);  // set I2C pins [SDA = D6, SCL = D5], default clock is 100kHz
  rtc.begin();         // initialize RTC chip
  
  //EEPROM.begin(EEPROM_SIZE);
  //GetCfgFromEEPROM();
}
/********************************* END OF SETUP *************************************/

// a small function for button1 (B1) debounce
bool debounce2()
{
  byte count = 0;
  for (byte i = 0; i < 5; i++)
  {
    if (digitalRead(button2))
      count++;
    delay(10);
  }

  if (count > 2)  return 1;
  else           return 0;
}



/******************************************************
 ******************************************************/
// a small function for button1 (B1) debounce
bool debounce ()
{
  byte count = 0;
  for (byte i = 0; i < 5; i++)
  {
    if (digitalRead(button1))
      count++;
    delay(10);
  }

  if (count > 2)  return 1;
  else           return 0;
}



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
    for (uint8_t RTCount = 0 ; RTCount < RUNPERDAY; RTCount++ )
    {
      RunTimeFlag [StCount][RTCount] = 0; //Reset Run Times flag in order to redisplay
    }

  }
}
/************************** END of Basic Display Function ************************/

/**********************************************************************
 *                                                                    *
                      Display Configuration
 *                                                                    *
 **********************************************************************/
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
      display.print("Watering Freq");
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


/**********************************************************************
 *                                                                    *
                      Display Date Time
 *                                                                    *
 **********************************************************************/
void RTC_display()
{
  char dow_matrix[7][10] = {"SUNDAY", "MONDAY", "TUESDAY", "WEDNESDAY", "THURSDAY", "FRIDAY", "SATURDAY"};
  byte x_pos[7] = {24, 24, 21, 15, 18, 24, 18};
  static byte previous_dow = 8;

  // print day of the week
  if ( previous_dow != now.dayOfTheWeek() )
  {
    previous_dow = now.dayOfTheWeek();
    display.fillRect(15, 0, 54, 8, WHITE);     // draw rectangle (erase day from the display)
  }
  display.setCursor(x_pos[previous_dow], 0);
  display.print( dow_matrix[now.dayOfTheWeek()] );

  // print date
  display.setCursor(12, 8);
  display.printf( "%02u-%02u-%04u", now.day(), now.month(), now.year() );
  // print time
  display.setCursor(28, 16);
  display.printf( "%02u:%02u:%02u", now.hour(), now.minute(), now.second() );
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
    while ( digitalRead(button2) )
    { // while B2 is pressed
      //while ( debounce2() );  //wait for beginning ripples
      delay(50);
      Display_value ^= 1; //convert between 0 , 1 to give values "*" or "o"
      if (Display_value == 0)   //every press it convert if 0 --> "o"
        Parameter = Lowercase_o;   //Display to user Lowercase o mean not selected
      else
        Parameter = _Asterix;   //Display to user Asterix mean selected

      display.setCursor(X_dowPos[x_axis], Y_dowPos[y_axis]);
      display.printf("%c", Parameter);
      display.display();  // update the screen
      //while( debounce2() );         // wait until button 2 (Increment) Released
      delay(300);
    }

    //flashing

    display.fillRect(X_dowPos[x_axis], Y_dowPos[y_axis], 11, 8, WHITE);
    display.display();  // update the screen
    unsigned long previous_m = millis();
    while ( (millis() - previous_m < 280) && !digitalRead(button1) && !digitalRead(button2)) ;
    display.setCursor(X_dowPos[x_axis], Y_dowPos[y_axis]);
    display.printf("%c", Parameter);
    display.display();  // update the screen
    previous_m = millis();
    while ( (millis() - previous_m < 280) && !digitalRead(button1) && !digitalRead(button2)) ;

    //if Button B1 is pressed (Transfer button)
    if (digitalRead(button1))
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
void DOW_valDisplay(char  *Parameter)
{
  static byte X_dowPos[2] = {42, 0} , Y_dowPos[4] = {0, 8, 16, 24};
  static byte x_axis = 1, y_axis = 0 ;

  if (DOW_Cntr == 0)
  {
    x_axis = 1;
    y_axis = 0;
  }

  display.setCursor(X_dowPos[x_axis], Y_dowPos[y_axis]);
  display.printf("%c", *Parameter);
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

  while (!(digitalRead(button3)) )
  { //while Exit button did not pressed increment and continue
    while (digitalRead(button2)) { // while B2 is pressed
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
    while ( (millis() - previous_m < 280) && !digitalRead(button1) && !digitalRead(button2)) ;
    display.setCursor(x_pos[i], y_pos);
    display.printf("%02u", parameter);
    display.display();  // update the screen
    previous_m = millis();
    while ( (millis() - previous_m < 280) && !digitalRead(button1) && !digitalRead(button2)) ;

    if (digitalRead(button1))
    { // if button B1 is pressed
      i = (i + 1) % 13;    // increment 'i' for the next parameter
      return parameter;   // return parameter value and exit
    }

  }//while(!Exit) END
  return parameter;
}
/*********************************** END OF EDIT FUNCTION ***********************************/

/*******************************************************************************************
                  Control_Station
********************************************************************************************
  INPUT: Station Number


*******************************************************************************************/
void Control_Station(uint8_t Station, uint8_t Status)
{
  switch (Station)
  {
    case First_Station:

      if (Status == OPEN)
      {
        //OpenStation1(); //open Desired station valve
        display.setCursor(0, 24);
        display.print("St. 1" );
        display.display();  // update the screen
      }
      else
      {
        //CloseStation1(); //Close Desired station valve
        display.fillRect(0, 24, 42, 8, WHITE);     // draw rectangle (erase day from the display)
        display.display();  // update the screen
      }

      break;

    case Second_Station:

      if (Status == OPEN)
      {
        //OpenStation2(); //open Desired station valve
        display.setCursor(43, 24);
        display.print("St. 2 ");
        display.display();  // update the screen
      }
      else
      {
        //CloseStation2(); //Close Desired station valve
        display.fillRect(43, 24, 42, 8, WHITE);     // draw rectangle (erase day from the display)
        display.display();  // update the screen
      }
      break;

    case Third_Station:

      if (Status == OPEN)
      {
        //OpenStation3(); //open Desired station valve
        display.setCursor(0, 32 );
        display.print("St. 3" );
        display.display();  // update the screen
      }
      else
      {
        //CloseStation3(); //Close Desired station valve
        display.fillRect(0, 32, 42, 8, WHITE);     // draw rectangle (erase day from the display)
        display.display();  // update the screen
      }
      break;

    case Fourth_Station:

      if (Status == OPEN)
      {
        //OpenStation4(); //open Desired station valve
        display.setCursor(43, 32);
        display.print("St. 4");
        display.display();  // update the screen
      }
      else
      {
        //CloseStation4(); //Close Desired station valve
        display.fillRect(43, 32, 42, 8, WHITE);     // draw rectangle (erase day from the display)
        display.display();  // update the screen
      }
      break;
  }
}
  //////////////////////////////////////////
 //              Main loop               //
//////////////////////////////////////////
void loop()
{

  //////////////////////////////////////////////////////////////////////////////////
  //             Configuration Edit Part                                          //
  //////////////////////////////////////////////////////////////////////////////////

  if ( digitalRead(button1) ) // if B1 is pressed (EDIT)
  {
    if ( debounce() )            // call debounce function (make sure B1 is pressed)
    {
      while ( debounce() ); // call debounce function (wait for B1 to be released)

      byte day    = edit( now.day() );          // edit date
      byte month  = edit( now.month() );        // edit month
      byte year   = edit( now.year() - 2000 );  // edit year
      byte hour   = edit( now.hour() );         // edit hours
      byte minute = edit( now.minute() );       // edit minutes

      // write time & date data to the RTC chip
      rtc.adjust(DateTime(2000 + year, month, day, hour, minute, 0));

      //Prompt user to enter Number of station
      NumOfStation = edit( NumOfStation );

      //Loop on station data
      for ( x = 0 ; x < NumOfStation ; x++)
      {
        if (Exit)
          break;

        ArrOfSt[x].StRunPeriod = edit(ArrOfSt[x].StRunPeriod);
        ArrOfSt[x].NumOfRunPerDay = edit(ArrOfSt[x].NumOfRunPerDay);
        //Loop on period start times
        for (uint8_t j = 0 ; j < ArrOfSt[x].NumOfRunPerDay ; j++)
        {
          if (Exit)
            break;

          if (j == 0)
            ArrOfSt[x].RunTime1 = edit(ArrOfSt[x].RunTime1);
          else if (j == 1)
            ArrOfSt[x].RunTime2 = edit(ArrOfSt[x].RunTime2);
          else if (j == 2)
            ArrOfSt[x].RunTime3 = edit(ArrOfSt[x].RunTime3);
          else if (j == 3)
            ArrOfSt[x].RunTime4 = edit(ArrOfSt[x].RunTime4);
          else if (j == 4)
            ArrOfSt[x].RunTime5 = edit(ArrOfSt[x].RunTime5);
        }//Run Times Loop
        
        i = 6;  //back to station period step


        //////////obtain watering days of week////////////////////////////
        /*Call the next page which is DOW_To_Run Select*/
        Config_Display(DOW_Display);

        if (Flag == 0 ) //First Run of program Reset all Run Times selections
        {
          strcpy( ArrOfSt[0].DOW_Run , "ooooooo");
          strcpy( ArrOfSt[1].DOW_Run , "ooooooo");
          strcpy( ArrOfSt[2].DOW_Run , "ooooooo");
          strcpy( ArrOfSt[3].DOW_Run , "ooooooo");

          Flag = 1;  // Set the flag of the initial values of days of Run after the loop of stations
        }

        /* Display firstly values of Days of weeks selections*/
        for (uint8_t n = 0 ; n < WeeKNofDays ; n++)
        {

          DOW_valDisplay(&ArrOfSt[x].DOW_Run[n]);

          if (Exit)   //any time exit button pressed break from the loop
          {
            DOW_Cntr = 0;
            break;
          }    // break from the loop

        }
        for (uint8_t n = 0 ; n < WeeKNofDays ; n++)
        {
          ArrOfSt[x].DOW_Run[n] = DOW_edit(ArrOfSt[x].DOW_Run[n]);

          if (Exit)
          {
            DOW_Cntr = 0; //Reset Counter
            break;
          }
        }
      }//End For loop NumOfStation

      while (debounce()); // call debounce function (wait for button B1 to be released)

      i = 0;      // Reset i to return display cursor to position {0,0}

      Basic_Display();

      //SaveCfgToEEPROM();

    }//End of EDIT

  }
  //////////////////////////////////////// END of Edit /////////////////////////////////////////////////////


  /////////////////////////////////////// Display Time /////////////////////////////////////////////////////
  now = rtc.now();  // read current time and date from the RTC chip



  RTC_display();   // display time & calendar
  delay(100);      // wait 100 ms

//  display.setCursor(0, 34 );
//  display.print("St. 3" );
//  display.display();  // update the screen
//
//  display.setCursor(43, 34 );
//  display.print("St. 4" );
//  display.display();  // update the screen




  /********************************************************************************************
                    Apply Configuration Part
   ********************************************************************************************
     Read Days of week to Run in each station
     check how many station
     check watering days of each station
     in each day check Run Times per day
     open each station according to Time of Run
  */

 
  //Loop on Number of Station
  for ( uint8_t St_Num = 0 ; St_Num < NumOfStation ; St_Num++)
  {
      /* check This Day is wtering Day */
    if ( (ArrOfSt[St_Num].DOW_Run[now.day()]) == _Asterix)
    {
      display.setCursor(43, 40);
      display.printf("%02u", NumOfStation );
      display.display();  // update the screen
    
      display.setCursor(0, 40);
      display.printf("%02u", St_Num);
      display.display();  // update the screen

      delay(1000);

      
      /* Therefore this is the day of watering to this station */
      /* Next question when we Run is how many times per day */

      //Loop on Number of run per day to this station
      for (uint8_t RunTnum = 0 ; RunTnum < ArrOfSt[St_Num].NumOfRunPerDay ; RunTnum++)
      {
        if (RunTnum == 0)
        {
          if ( ((now.hour()) == ArrOfSt[St_Num].RunTime1) && ( (now.minute()) < ArrOfSt[St_Num].StRunPeriod) )
          { //This is the Time to open the required station
            Control_Station(St_Num, OPEN);  //Open it
          }
          else
          {
            Control_Station(St_Num, CLOSE); //Close Station
          }
        }

        else if (RunTnum == 1)
        {
          if ( ((now.hour()) == ArrOfSt[St_Num].RunTime2) && ( (now.minute()) < ArrOfSt[St_Num].StRunPeriod )  )
          { //This is the Time to open the required station
            Control_Station(St_Num, OPEN);  //Open it
          }
          else 
          {
            Control_Station(St_Num, CLOSE); //Close Station
          }
        }


        else if (RunTnum == 2)
        {
          if ( ((now.hour()) == ArrOfSt[St_Num].RunTime3) && ( (now.minute()) < ArrOfSt[St_Num].StRunPeriod )  )
          {
            Control_Station(St_Num, OPEN);  //Open it
          }
          else 
          { 
            Control_Station(St_Num, CLOSE); //Close Station
          }
        }


        else if (RunTnum == 3)
        {
          if ( ((now.hour()) == ArrOfSt[St_Num].RunTime4) && ( (now.minute()) < ArrOfSt[St_Num].StRunPeriod )  )
          { //This is the Time to open the required station
            Control_Station(St_Num, OPEN);  //Open it
          }
          else
          { 
            Control_Station(St_Num, CLOSE); //Close Station
          }
        }

        else if (RunTnum == 4)
        {
          if ( ((now.hour()) == ArrOfSt[St_Num].RunTime5) && ( (now.minute()) < ArrOfSt[St_Num].StRunPeriod )  )
          { //This is the Time to open the required station
            Control_Station(St_Num, OPEN);  //Open it
          }
          else
          {
            Control_Station(St_Num, CLOSE); //Close Station
          }
        }
      }//For loop on Run Times of each station
    }//If Day of Run
  }//For loop on station
  

}// end of code.

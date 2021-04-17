#include <Wire.h>
#define I2CAddressESPWifi 6
int x=1;

void setup()
{
  Serial.begin(115200);
  Wire.begin(0, 1); //Change to Wire.begin() for non ESP.
  delay(2000);
}

void loop()
{
  while(x<5)
  { 
    delay(5000); 
    Wire.beginTransmission(I2CAddressESPWifi);
    Wire.write(x);
    Wire.endTransmission();
    x++;
  
    delay(10);//Required. Not sure why!

    Wire.requestFrom(I2CAddressESPWifi,1);
    Serial.print("Request Return:[");
    while (Wire.available())
    {
      delay(100);
      char c = Wire.read();
      Serial.print(c);
    }
    Serial.println("]");
    
   }
}

#include <Wire.h> 
#define BLYNK_PRINT Serial
#include <SPI.h>
#include <Ethernet.h>
//#include <BlynkSimpleEsp32.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>


char auth[] = "SCsUidsxfKCvfrQiqZ6UoOGbUa_ncH4K";
char ssid[] = "fsp";
char pass[] = "9846791147";
#define relaypin D1
//#define TdsSensorPin 27

#define SERIAL Serial
#define sensorPin A0

int sensorValue = 0;
float tdsValue = 0;
float Voltage = 0;

void checkButtonState()
{
    
    sensorValue = analogRead(sensorPin);
    Voltage = sensorValue*5/1024.0; //Convert analog reading to Voltage
    tdsValue=(133.42/Voltage*Voltage*Voltage - 255.86*Voltage*Voltage + 857.39*Voltage)*0.5; //Convert voltage value to TDS value
    SERIAL.print("TDS Value = "); 
    SERIAL.print(tdsValue);
    SERIAL.println(" ppm");
    delay(1000);
 
 
       Blynk.virtualWrite(V1, tdsValue);
    
    
  
 
}



BLYNK_WRITE(V0)
{
int    pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
 if (pinValue == 1)
  {
  digitalWrite(relaypin, LOW);
  delay(5000);
    digitalWrite(relaypin, HIGH);
    
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(relaypin, OUTPUT);
//  pinMode(TdsSensorPin,INPUT);
   digitalWrite(relaypin, HIGH);
   
 

  
  Blynk.begin(auth, ssid, pass,"blynk.cloud");
  // timer.setInterval(5000L, checkButtonState);
}

void loop() {
checkButtonState();

   Blynk.run();
  //timer.run();
}
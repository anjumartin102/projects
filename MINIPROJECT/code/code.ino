#define BLYNK_TEMPLATE_ID     "*****"
#define BLYNK_DEVICE_NAME     "ASUS_VB"
#define BLYNK_AUTH_TOKEN      "*****"
#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Esp32Servo.h>
char auth[] = "BLYNK_AUTH_TOKEN";
char ssid[] = "ssid";
char pass[] = "password";
BlynkTimer timer;

Servo servo;
#define TdsSensorPin 27
#define VREF 3.3              // analog reference voltage of the ADC
#define SCOUNT  30            // sum of sample point
int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;       // current temperature for compensation

void myTimer() 
{
    Blynk.virtualWrite(V0, tdsValue);  
}

void setup() {
  Serial.begin(115200);
  //Connecting to Blynk Cloud
  Blynk.begin(auth, ssid, pass); 

  servo1.attach(A2); // Attach servo 1 to GPIO pin. Change it
  
    timer.setInterval(1000L, myTimer); 
}

void loop() {
  tds_val = analogRead(A0); 
  // Runs all Blynk stuff
  Blynk.run(); 
  // runs BlynkTimer
  timer.run();
}
BLYNK_WRITE(V0) // Slider Widget for Servo 1 on V0
{
  int pos1 = param.asInt(); // Get value from switchslider
  servo1.write(180); // Set servo 1 position
  servo1.write(0);
}
// median filtering algorithm
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0){
    bTemp = bTab[(iFilterLen - 1) / 2];
  }
  else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}

void tds_sensor()
{
  static unsigned long analogSampleTimepoint = millis();
  if(millis()-analogSampleTimepoint > 40U){     
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
  }   
 
  static unsigned long printTimepoint = millis();
  if(millis()-printTimepoint > 800U){
    printTimepoint = millis();
    for(copyIndex=0; copyIndex<SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      
            averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0;
      
      //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0)); 
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      //temperature compensation
      float compensationVoltage=averageVoltage/compensationCoefficient;
      
      //convert voltage value to tds value
      tdsValue=(133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      Serial.print("TDS Value:");
      Serial.print(tdsValue,0);
      Serial.println("ppm");
    }
  }
}

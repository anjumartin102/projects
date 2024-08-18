#define BLYNK_TEMPLATE_ID "TMPL3BEAjFFK9"
#define BLYNK_TEMPLATE_NAME "Control Servo Motor With ESP32"
#define BLYNK_AUTH_TOKEN "4sgophsqO_KMv8dfL7aPgRZiTkdsQdVigg_Xlszw"

#define BLYNK_PRINT Serial
#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <ESP32Servo.h>


char auth[] = BLYNK_AUTH_TOKEN; // Blynk authentication token
char ssid[] = "WiFi Username"; // Your WiFi SSID
char pass[] = "WiFi Password"; 

Servo servo1;
#define TdsSensorPin 27
#define VREF 3.3              // analog reference voltage of the ADC
#define SCOUNT  30            // sum of sample point
int analogBuffer[SCOUNT];     // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0;
int copyIndex = 0;
float averageVoltage = 0;
float tdsValue = 0;
float temperature = 25;

void myTimerEvent()
{
  Blynk.virtualWrite(V21, millis()/1000); //reset med
}

BlynkTimer timer;

void setup()
{
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);
  servo1.attach(2); // Attach servo 1 to GPIO 2
  pinMode(TdsSensorPin, INPUT);
  //sensors.begin();

   timer.setInterval(1000L, myTimerEvent); 
}

void loop()
{
   tds_sensor();
   Blynk.run();
   timer.run();
}

BLYNK_WRITE(V0) // Slider Widget for Servo 1 on V0
{
  int pos1 = param.asInt(); // Get value from slider
  servo1.write(pos1); // Set servo 1 position
  Blynk.virtualWrite(V2, pos1);
}
int getMedianNum(int bArray[], int iFilterLen)
{
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++)
    bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++)
  {
    for (i = 0; i < iFilterLen - j - 1; i++)
    {
      if (bTab[i] > bTab[i + 1])
      {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
  else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  return bTemp;
}


void tds_sensor()
{
  
  static unsigned long analogSampleTimepoint = millis();
  if (millis() - analogSampleTimepoint > 40U)  //every 40 milliseconds,read the analog value from the ADC
  {
    analogSampleTimepoint = millis();
    analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if (analogBufferIndex == SCOUNT)
      analogBufferIndex = 0;
  }
  static unsigned long printTimepoint = millis();
  if (millis() - printTimepoint > 800U)
  {
    printTimepoint = millis();
    for (copyIndex = 0; copyIndex < SCOUNT; copyIndex++)
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
    averageVoltage = getMedianNum(analogBufferTemp, SCOUNT) * (float)VREF / 1024.0; // read the analog value more stable by the median filtering algorithm, and convert to voltage value
    float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0); //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
    float compensationVolatge = averageVoltage / compensationCoefficient; //temperature compensation
    tdsValue = (133.42 * compensationVolatge * compensationVolatge * compensationVolatge - 255.86 * compensationVolatge * compensationVolatge + 857.39 * compensationVolatge) * 0.5; //convert voltage value to tds value
 
    Serial.print("TDS Value:");
    Serial.print(tdsValue, 0);
    Serial.println("ppm");
 
    
  }
}
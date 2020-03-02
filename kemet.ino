#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
BlynkTimer timer;
#define BLYNK_PRINT Serial

char auth[] = "Y4GKeusFpVLXHpN7nJFknjJxxxxxx";
char ssid[] = "xxx";
char pass[] = "xxxxxx";

unsigned long PyroRead = 0;
int Pyro=5;  //D1 of MCU
unsigned long IR_threshold = 200000;
int state=1;
int IR_sensed = 0;
int relay=16; //D0 of MCU

int Sensor_Pin = A0;//current sensor
unsigned int Sensitivity = 185;   // 185mV/A for 5A, 100 mV/A for 20A and 66mV/A for 30A Module
float Vpp = 0; //
float Vrms = 0; // rms voltage
float Irms = 0; // rms current
float Supply_Voltage = 230.0;    
float Vcc = 3.0;         // ADC reference voltage // voltage at 5V pin 
float power = 0;         // power in watt             
unsigned long last_time =0;
unsigned long current_time =0;
unsigned long interval = 100;
unsigned int calibration = 100; 
unsigned int pF = 85;    
float Wh =0 ;
float bill_amount = 0; 
unsigned int energyTariff = 5.0;

 
void setup() {
pinMode (relay, OUTPUT); //relay
pinMode (Pyro,INPUT); //kemet sensor
pinMode(Sensor_Pin,INPUT);//sensor is connected to A0
Serial.begin(9600);
Blynk.begin(auth, ssid, pass);
}
BLYNK_WRITE(V4) {  // calibration slider 
    calibration = param.asInt();}
void loop() {
timer.run();
while ((IR_sensed < 2))
{  Blynk.run();
   PyroRead = pulseIn(Pyro, HIGH);
   if(PyroRead > IR_threshold)
   {
      IR_sensed++;
   }
}   
digitalWrite(relay,state);

if(state==1)
{
  Serial.println("HIGH");
  Vpp = getVPP();
  Vrms = (Vpp/2.0) *0.707; 
  Vrms = Vrms - (calibration / 10000.0); 
  Irms = (Vrms * 1000)/Sensitivity ;
  if((Irms > -0.015) && (Irms < 0.008)){  // remove low end chatter
    Irms = 0.0;
  }
  power= (Supply_Voltage * Irms) * (pF / 100.0); 
  Serial.println(power);
  last_time = current_time;
  current_time = millis();    
  
 Wh = Wh+  power *(( current_time -last_time) /3600000.0) ; // calculating energy in Watt-Hour
 bill_amount = Wh*(energyTariff/1000);
 Blynk.virtualWrite(V2,"ON");
}
if(state==0)
{
  Serial.println("LOW");
  Serial.println(bill_amount);
  Blynk.virtualWrite(V1, bill_amount);
  Blynk.virtualWrite(V2,"OFF") ;
}

state=1-state;  
PyroRead = 0;
IR_sensed = 0;
delay(1000); 
}
float getVPP()
{
  float result; 
  int readValue;                
  int maxValue = 0;             
  int minValue = 1024;          
  uint32_t start_time = millis();
  while((millis()-start_time) < 950) 
//read every 0.95 Sec
  {
     readValue = analogRead(Sensor_Pin);   
     if (readValue > maxValue) 
     {         
         maxValue = readValue; 
     }
     if (readValue < minValue) 
     {          
         minValue = readValue;
     }
  } 
   result = ((maxValue - minValue) * Vcc) / 1024.0;  
   return result;
 }






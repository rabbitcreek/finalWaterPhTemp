/*************************************************************
  Download latest Blynk library here:
    https://github.com/blynkkk/blynk-library/releases/latest

  Blynk is a platform with iOS and Android apps to control
  Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build graphic interfaces for all your
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: http://www.blynk.cc
    Sketch generator:           http://examples.blynk.cc
    Blynk community:            http://community.blynk.cc
    Follow us:                  http://www.fb.com/blynkapp
                                http://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************

  This example shows how value can be pushed from Arduino to
  the Blynk App.

  NOTE:
  BlynkTimer provides SimpleTimer functionality:
    http://playground.arduino.cc/Code/SimpleTimer

  App project setup:
    Value Display widget attached to Virtual Pin V5
 *************************************************************/

/* Comment this out to disable prints and save space */
float tempTime = 0;
float timerOne = 0;
float sprayTime = 0;
bool state = 0;
unsigned long int avgValue;  //Store the average value of the sensor feedback
float b;
int buf[10],temp;
#define acidValue 2320  //This is the raw sensor value for the pH buffer at 4.0
#define neutralValue 1660 //This is the raw sensor value for the pH buffer at 7.0
#define reader 39  // This is the GPIO pin for the pH meter
#include <Wire.h>
#include "SSD1306Ascii.h" // https://github.com/greiman/SSD1306Ascii
#include "SSD1306AsciiWire.h" // https://github.com/greiman/SSD1306Ascii
#define BLYNK_PRINT Serial
#define NUMSAMPLES 5
// the value of the 'other' resistor
#define SERIESRESISTOR 1783    //This is the value of the resistor in the eTape
#define I2C_ADDRESS 0x3C
#include <OneWire.h> //required to read temperature
#include <DallasTemperature.h>  //required to read temperature
float acidC = (acidValue -1500)/3;  //easier to use values in the formula
float neutC = (neutralValue - 1500)/3;
float slope = 3/(neutC - acidC);  //calculate slope of the pH curve
float  interCept = 7 - (slope * neutC);  //calculate y--intercept for the formula y = mx + b where m is slope and b is y intercept
#define ONE_WIRE_BUS 15   //temp reading is on this GPIO pin

OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

 float Celcius=0;
 float Fahrenheit=0;
SSD1306AsciiWire oled;  //for oled screen
// What pin to connect the sensor to
  
 
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
int inches;
int samples[NUMSAMPLES];

// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
char auth[] = "place toke here";


// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "xxxxxx";  //what wifi network name
char pass[] = "xxxxxxxxxxx"; // wap signin for network
//set-up various timers for the Blynk app
BlynkTimer timer_pH;
BlynkTimer timer_temp;
BlynkTimer timer_water;
BlynkTimer timer_valve;




int getInches(){
  float average;
  uint8_t i;
  // take N samples in a row, with a slight delay
  for (i=0; i< NUMSAMPLES; i++) {
   samples[i] = analogRead(34);
   Serial.print("raw reading");
   Serial.println(samples[i]);
   delay(10);
  }
 
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += samples[i];
  }
  average /= NUMSAMPLES;
 
  //Serial.print("Average analog reading "); 
  //Serial.println(average);
  // convert the value to resistance
  average = 4095 / average - 1;
  average = SERIESRESISTOR / average;
 
  Serial.print("Thermistor resistance "); 
  Serial.println(average);
  average = constrain(average,580,1520);
 inches = map(average,1520,580,2,10);
 Serial.print("inches of water");
 Serial.println(inches);
  oled.clear();
  
  oled.home();
  oled.set2X();
  oled.println("  Water");
  oled.print("    ");
  oled.println(inches);
  oled.println("  Inches");
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
 return inches;
}
void mypHEvent()
{
  
  for(int i=0;i<10;i++)       //Get 10 sample value from the sensor for smooth the value
  { 
    buf[i]=analogRead(reader);
    delay(10);
  }
  for(int i=0;i<9;i++)        //sort the analog from small to large
  {
    for(int j=i+1;j<10;j++)
    {
      if(buf[i]>buf[j])
      {
        temp=buf[i];
        buf[i]=buf[j];
        buf[j]=temp;
      }
    }
  }
  avgValue=0;
  for(int i=2;i<8;i++)                      //take the average value of 6 center sample
    avgValue+=buf[i];
  float phValue=(float)avgValue/6; //convert the analog into millivolt
  float phValue2 = phValue;
  phValue=(phValue - 1500)/3; 
  
  
  //convert the millivolt into pH value
  phValue = (slope * phValue) +interCept;
  Serial.print("    pH:");  
  Serial.print(phValue,2);
  Serial.println(" ");
  Serial.print(phValue2,2);
  oled.clear();
  
  oled.home();
  oled.println("  pH");
  oled.print("    ");
  oled.println(phValue,2);
  //oled.println("  Inches");
  Blynk.virtualWrite(V7,phValue);
  
}
void myValveEvent(){
  Blynk.virtualWrite(V8,sprayTime);
  
}
void myTempEvent()
{
  sensors.requestTemperatures(); 
  Celcius=sensors.getTempCByIndex(0);
  Fahrenheit=sensors.toFahrenheit(Celcius);
  Serial.print(" C  ");
  Serial.print(Celcius);
  Serial.print(" F  ");
  Serial.println(Fahrenheit);
   oled.clear();
  
  oled.home();
  oled.set2X();
  oled.println(" Temp");
  oled.print("    ");
  oled.println(Fahrenheit);
  oled.println("DegreesF");
  Blynk.virtualWrite(V6,Fahrenheit);
}
void myWaterEvent()
{

  
  Blynk.virtualWrite(V5, getInches());
  
}

void setup()
{
  // Debug console
  pinMode(32, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
  pinMode(27, OUTPUT);
  pinMode(33, OUTPUT);
  digitalWrite(27, LOW);
  digitalWrite(33, LOW); 
  
  Serial.begin(115200);
  sensors.begin();
  Wire.begin(); 
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.set400kHz();  
  oled.setFont(Adafruit5x7); 
  oled.setScroll(true);   
  oled.clear();
  oled.home();
  oled.set2X();
  Blynk.begin(auth, ssid, pass);
  digitalWrite(27, HIGH);
  // You can also specify server:
  //Blynk.begin(auth, ssid, pass, "blynk-cloud.com", 80);
  //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

  // Setup a function to be called every second
  timer_pH.setInterval(25000L, mypHEvent);
  timer_temp.setInterval(35000L, myTempEvent);
  timer_water.setInterval(45000L, myWaterEvent);
  timer_valve.setInterval(10000L, myValveEvent);
}

void loop()
{
  //Serial.print("inches");
  //Serial.println(getInches());
  Blynk.run();
  timer_pH.run(); // Initiates BlynkTimer
  timer_temp.run();
  timer_water.run();
  timer_valve.run();
  bool sw2 = digitalRead(32);
  delay(50);
  bool sw1 = digitalRead(14);
  delay(50);
  if (!sw1 & !sw2) {
  digitalWrite(33, HIGH);
  if(!state){
    timerOne = millis();
    state = 1;
  }
  }
 
  else if(sw1 & sw2){
  digitalWrite(33,LOW);
  if(state){
     sprayTime = (millis() - timerOne)/1000.0;
    state = 0;
  }
  }
  //getInches();
}


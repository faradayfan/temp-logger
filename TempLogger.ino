#include <Wire.h>
#include "RTClib.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define DHTTYPE DHT11
#define DHTPIN 11
#define BUTTON_A 9 // delete file
#define BUTTON_B 6 
#define BUTTON_C 5

const int chipSelect = 4;
const double BAD_TEMP = -999999;
const double BAD_HUMIDITY = -999999;
const int STARTUP_WAIT_TIME = 5000;
const int POLLING_TIME = 5000;
const String FILENAME = "templog.txt";

RTC_DS3231 rtc;
DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display = Adafruit_SSD1306();
long epoch = 0;
bool button_pressed = false;

String ISODateTime();
double getHumidity();
double getTemp();
void loggerWriteLine(String line);

void setup () {
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);
  
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    while (1);
  }

  Serial.begin(9600);

  delay(STARTUP_WAIT_TIME); // wait for console opening

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (rtc.lostPower()) {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  display.clearDisplay();
  display.display();
  // text display tests
  display.setTextSize(1);
  display.setTextColor(WHITE);
  epoch = rtc.now().unixtime() - POLLING_TIME;
}

void loop () {

  sensors_event_t event;  
  dht.temperature().getEvent(&event);

  String timestamp = "";
  double temp = 0;
  double humi = 0;

  timestamp = ISODateTime();
  temp = getTemp();
  humi = getHumidity();

  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(timestamp);
  display.println("Temp: " + (String)temp + "C " + (String)(((9 * temp)/5)+ 32) + "F");
  display.println("RH:   " + (String)humi + "%");
  display.display();
  
  String data = timestamp + ", " + temp + ", " + (((9 * temp)/5)+ 32) + ", "+ humi + ", ";

  loggerWriteLine(data);
    
  delay(POLLING_TIME);
  
}

double getTemp(){
  sensors_event_t event;  
  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature))
    return event.temperature;
  else {
    Serial.println("Bad Temp Poll");
    return BAD_TEMP;
  }
    
}

double getHumidity(){
  sensors_event_t event;  
  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity)) {
    return event.relative_humidity;
  } else {
    Serial.println("Bad Temp Poll");
    return BAD_HUMIDITY;
  }   
}

void loggerWriteLine(String line){
  File dataFile = SD.open(FILENAME, FILE_WRITE);

  if (dataFile) {
    dataFile.println(line);
    dataFile.close();
    Serial.println("Line written: '" + line + "' to file: " + FILENAME);
  } else {
    Serial.println("error opening datalog.txt");
  }
}

String ISODateTime(){
  DateTime now = rtc.now();
  return (String)"" + now.year() + "-" + now.month() + "-" + now.day() + "T" + now.hour() + ":" + now.minute() + ":" + now.second();
}



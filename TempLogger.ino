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
const String fileprefix = "temperature_log";

RTC_DS3231 rtc;
DHT_Unified dht(DHTPIN, DHTTYPE);
Adafruit_SSD1306 display = Adafruit_SSD1306();
long epoch = 0;
bool button_pressed = false;

// Function declarations
void setupDisplay();
void setupRTC();
void setupSDCard();
void setupSensors();
String getDate();
String getTime();
double getHumidity();
double getTemp();
void updateDisplay(String datetime, double temp, double humidity);
void loggerWriteLine(String filename, String date, String time, double temp, double humidity);

void setup()
{
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);

  Serial.begin(9600);

  delay(STARTUP_WAIT_TIME); // wait for console opening

  setupSensors();
  setupRTC();
  setupDisplay();
  setupSDCard();
}

void loop()
{

  String date = "";
  String time = "";
  double temp = 0;
  double humi = 0;

  date = getDate();
  time = getTime();
  temp = getTemp();
  humi = getHumidity();

  updateDisplay(date + " " + time, temp, humi);

  loggerWriteLine(fileprefix + ".csv", date, time, temp, humi);

  delay(POLLING_TIME);
}

void setupRTC()
{
  if (!rtc.begin())
  {
    Serial.println("Couldn't find RTC");
    while (1)
      ;
  }

  if (rtc.lostPower())
  {
    Serial.println("RTC lost power, lets set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

void setupDisplay()
{
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.display();
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void setupSDCard()
{
  if (!SD.begin(chipSelect))
  {
    Serial.println("Card failed, or not present");
    while (1)
      ;
  }
}

void setupSensors()
{
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
}

String getDate()
{
  DateTime now = rtc.now();
  return (String) "" + now.month() + "/" + now.day() + "/" + +now.year();
}

String getTime()
{
  DateTime now = rtc.now();
  return (String) "" + now.hour() + ":" + now.minute() + ":" + now.second();
}

double getTemp()
{
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (!isnan(event.temperature))
    return event.temperature;
  else
  {
    Serial.println("Bad Temp Poll");
    return BAD_TEMP;
  }
}

double getHumidity()
{
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (!isnan(event.relative_humidity))
  {
    return event.relative_humidity;
  }
  else
  {
    Serial.println("Bad Temp Poll");
    return BAD_HUMIDITY;
  }
}

void updateDisplay(String datetime, double temp, double humidity)
{
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(datetime);
  display.println("Temp: " + (String)temp + "C " + (String)(((9 * temp) / 5) + 32) + "F");
  display.println("RH:   " + (String)humidity + "%");
  display.display();
}

void loggerWriteLine(String filename, String date, String time, double temp, double humidity)
{
  String line = date + ", " + time + ", " + ", " + temp + ", " + (((9 * temp) / 5) + 32) + ", " + humidity;

  File dataFile = SD.open(filename, FILE_WRITE);

  if (dataFile)
  {
    dataFile.println(line);
    dataFile.close();
    Serial.println("Line written: '" + line + "' to file: " + fileprefix);
  }
  else
  {
    Serial.println("error opening " + filename);
  }
}

// REST Temperature
// Features:
// 1. T-Display

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();




//https://randomnerdtutorials.com/esp32-date-time-ntp-client-server-arduino/
//https://www.survivingwithandroid.com/esp32-rest-api-esp32-api-server/
//https://www.mischianti.org/2020/05/24/rest-server-on-esp8266-and-esp32-get-and-json-formatter-part-2/
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include "time.h"
const char* ntpServer = "pool.ntp.org";
//const long  gmtOffset_sec = 3600 * 4;
const long  gmtOffset_sec = 3600 * -8;
const int   daylightOffset_sec = 3600;

#define MAX_ELEMENTS 427 // 428 -> Crash

#define BME_SCK 13
#define BME_MISO 12
#define BME_MOSI 11
#define BME_CS 10
#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME680 bme; // I2C
const char *SSID = "Hunter";
const char *PWD = "saturday";
const int red_pin = 5;   
const int green_pin = 18; 
const int blue_pin = 19; 
// Setting PWM frequency, channels and bit resolution
const int frequency = 5000;
const int redChannel = 0;
const int greenChannel = 1;
const int blueChannel = 2;
const int resolution = 8;
int SAMPLE_INTERVAL = 10 * 1000;

WebServer server(80);
StaticJsonDocument<250> jsonDocument;
StaticJsonDocument<34000> jsonDocument_temperature;
char buffer[34000];
float temperature;
float humidity;
float pressure;
String printLocalTime();
void setup_task();
void getMenu();
void getData();
void resetData();
void getTemperature();
void getPressure();
void getHumidity();
void getSettings();
void setup_routing();
void read_sensor_data(void * parameter);

void setup()
{
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLUE);

  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 async test"));

  if (!bme.begin()) {
    Serial.println(F("Could not find a valid BME680 sensor, check wiring!"));
    while (1);
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  Serial.print("Connecting to Wi-Fi");
  WiFi.begin(SSID, PWD);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);

  // Init and get the time
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  printLocalTime();
  }
 
  Serial.print("Connected! IP Address: ");
  Serial.println(WiFi.localIP());
  setup_task();
  setup_routing();
}


void setup_task()
{
  xTaskCreate(
  read_sensor_data,
  "Read sensor data",
  2000,
  NULL,
  1,
  NULL
  );
}


void setup_routing()
{
  server.on("/", getMenu);
  server.on("/temperature", getTemperature);
  server.on("/pressure", getPressure);
  server.on("/humidity", getHumidity);
  server.on("/data", getData);
  //  server.on("/led", HTTP_POST, handlePost);
  server.on("/reset", resetData);
  server.on(F("/set"), HTTP_GET, getSettings);
  server.begin();
}



void getMenu()
{
    jsonDocument.clear();
    jsonDocument_temperature.clear();
    Serial.println("Data reset");

    DynamicJsonDocument doc1(2048);
    doc1["reset"] = "Great Success !";

    // Serialize JSON document
    String json;
    serializeJson(doc1, json);
    server.send(200, "application/html", json);
}



void getSettings()
{
    Serial.println("getSettings()");
    Serial.println( server.args() );

    String message = "";
    
    for ( uint8_t i = 0; i < server.args(); i++ )
    {
      message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
      String name1 = server.argName ( i );
      //      String value1 = server.arg ( i );
 
      if ( name1.equals("interval") )
      {
        SAMPLE_INTERVAL = server.arg ( i ).toInt();
        Serial.println( SAMPLE_INTERVAL );
      }
    }

    Serial.println( message );
    //    if (server.arg("interval")== "true")
    //    {
    //        response+= ",\"signalStrengh\": \""+String(WiFi.RSSI())+"\"";
    //    }
    DynamicJsonDocument doc1(2048);
    doc1["set"] = "Great Success !";

    // Serialize JSON document
    String json;
    serializeJson(doc1, json);
    server.send(200, "application/json", json);
}

void create_json(char *tag, float value, char *unit)
{
  jsonDocument.clear();
  jsonDocument["type"] = tag;
  jsonDocument["value"] = value;
  jsonDocument["unit"] = unit;
  serializeJson(jsonDocument, buffer);
}

void add_json_object(char *tag, float value, char *unit)
{

  JsonObject obj = jsonDocument.createNestedObject();
  obj["type"] = tag;
  obj["value"] = value;
  obj["unit"] = unit;

  int dic_size = jsonDocument_temperature.size();
  if (dic_size >= MAX_ELEMENTS)
  {
      Serial.println("Remove 1");
      jsonDocument_temperature.remove(0);
  }
  Serial.println( dic_size );

  JsonObject objt = jsonDocument_temperature.createNestedObject();
    objt["time"] = printLocalTime();
    //    objt["type"] = tag;
    objt["value"] = value;
    //    objt["unit"] = unit;
}








void loop_bme()
{
  unsigned long endTime = bme.beginReading();
  if (endTime == 0)
  {
    Serial.println(F("Failed to begin reading :("));
    return;
  }
  delay(50); // This represents parallel work.
  if (!bme.endReading())
  {
    Serial.println(F("Failed to complete reading :("));
    return;
  }
    float new_temperature = bme.temperature * 9 / 5 + 32;
//  Serial.print(F("Temperature = "));
//  Serial.print(temperature);
//  Serial.println(F(" *F"));

  pressure = bme.pressure / 100.0;
//  Serial.print(F("Pressure = "));
//  Serial.print( pressure );
//  Serial.println(F(" hPa"));
//
  humidity = bme.humidity;
//  Serial.print(F("Humidity = "));
//  Serial.print(humidity);
//  Serial.println(F(" %"));
//
//  Serial.print(F("Gas = "));
//  Serial.print(bme.gas_resistance / 1000.0);
//  Serial.println(F(" KOhms"));
//
//  Serial.print(F("Approx. Altitude = "));
//  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
//  Serial.println(F(" m"));

    if ( new_temperature < 350 )
    {
      temperature = new_temperature;
      add_json_object("temperature", temperature, "° F");
      tft.fillScreen(TFT_BLACK);
      tft.setTextColor(TFT_BLUE);
      // int t = temperature;
      String ts = String(temperature, 1);
      String F = "°";
      tft.drawString(ts, 10, 30, 8);
      tft.drawString("o", 200, 30, 4);
    }
}


void read_sensor_data(void * parameter)
{
   for (;;)
   {
      loop_bme();
      vTaskDelay(SAMPLE_INTERVAL / portTICK_PERIOD_MS);
   }
}


void resetData(){
    jsonDocument.clear();
    jsonDocument_temperature.clear();
    Serial.println("Data reset");

    DynamicJsonDocument doc1(2048);
    doc1["reset"] = "Great Success !";

    // Serialize JSON document
    String json;
    serializeJson(doc1, json);
    server.send(200, "application/json", json);
}

void getTemperature()
{
  Serial.println("Get temperature");
  serializeJson(jsonDocument_temperature, buffer);
  server.send(200, "application/json", buffer);
}


void getHumidity()
{
  // Serial.println("Get humidity");
  create_json("humidity", humidity, "%");
  server.send(200, "application/json", buffer);
}

void getPressure() {
  // Serial.println("Get pressure");
  create_json("pressure", pressure, "hPa");
  server.send(200, "application/json", buffer);
}

void getData()
{
  // Serial.println("Get BME280 Sensor Data");
  jsonDocument.clear();
  // add_json_object("temperature", temperature, "°C");
  add_json_object("temperature", temperature, "°F");
  add_json_object("humidity", humidity, "%");
  add_json_object("pressure", pressure, "hPa");
  serializeJson(jsonDocument, buffer);
  server.send(200, "application/json", buffer);
}

void handlePost()
{
  if (server.hasArg("plain") == false)
  {}
  String body = server.arg("plain");
  deserializeJson(jsonDocument, body);

  int red_value = jsonDocument["red"];
  int green_value = jsonDocument["green"];
  int blue_value = jsonDocument["blue"];

  ledcWrite(redChannel, red_value);
  ledcWrite(greenChannel,green_value);
  ledcWrite(blueChannel, blue_value);

  server.send(200, "application/json", "{}");
}


void loop()
{
  server.handleClient();
}


String printLocalTime()
{
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to obtain time");
    return "Error";
  }
  //  Serial.println(&timeinfo, "print - %A, %B %d %Y %H:%M:%S");

  //https://arduino.stackexchange.com/questions/52676/how-do-you-convert-a-formatted-print-statement-into-a-string-variable
  char timeStringBuff[50]; //50 chars should be enough
  strftime(timeStringBuff, sizeof(timeStringBuff), "%A, %B %d %Y %H:%M:%S", &timeinfo);
  //print like "const char*"
  //  Serial.println(timeStringBuff);
  //Optional: Construct String object 
  String asString(timeStringBuff);
  return asString;
}

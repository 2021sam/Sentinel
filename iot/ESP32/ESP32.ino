//10.27   1 Terminal

//  Sam Portillo
//  Features:
//    Terminal & MQTT
//  05/01/2021
//  05/26/2021  - TTGO T-Display
//  10/22/2021    Consolidated sensors
//  Board → TTGO T1 → i2c does not work.
//  Board → ESP32 Arduino  → TTGO LoRa32-OLED V1
//                          → M5Stack-Core-ESP32

#include "Terminal.h"
Terminal t;

int BUTTON_1 = 0;
int BUTTON_2 = 35;

//  Sensors
#include "PIR.h"
#include "LUX.h"
#include "sonic.h"
#include "Doors.h"

int PIR = 0;     //12;       // TTGO T-Display Works
int ENFORCER = 27;   // PIN=27  Seco-Larm E-931-S35RRQ Enforcer Indoor/Outdoor Wall Mounted Photoelectric Beam Sensor with 35 Foot Range
int LUX = 0;
int SONIC = 0;
int DOOR1 = 0;  //22;
int DOOR2 = 0;  //21;

String s;
uint16_t lux;
uint16_t llux = 0;
int ld = 0;
int shade = 20;
bool motion;
int sonic_value = 0;
unsigned long start_time = 0;
unsigned long last_time = 0;
int motion_count = 0;
unsigned long interval = 5000;      //  Allowed time to pass with no motion.
int motion_threshold = 1;

#include "IOT.h"                    //  10.22.2021 → Only variables above this are global

void setup()
{
  if ( t.device == 2 )
  {
      Serial.begin( 115200 );         // Need to comment out for M5Stack !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
      Serial.println("Starting...");
  }
  client.setServer( iot.mqtt_server, 1883);
  client.setCallback(callback);
  setup_sensors();
}


void setup_sensors()
{
  if ( t.device == 2 )
  {
      pinMode(BUTTON_1, INPUT);
      pinMode(BUTTON_2, INPUT);
  }

  if (PIR)
    pinMode(PIR, INPUT);

  if (ENFORCER)
    pinMode(ENFORCER, INPUT_PULLUP);       // Enforcer works beautifully with a INPUT_PULLUP

  if (DOOR1)
  {
    pinMode(DOOR1, INPUT);
    pinMode(DOOR2, INPUT);
  }

  if (LUX)
    setup_LUX();

  if (SONIC)
    setup_TOF();
}


void loop()
{
  checkConnections();
  t.checkFlash();
  read_sensors();
  delay( 100 );
}

void read_sensors()
{
  if ( t.device == 2 )
  {
      if ( ! digitalRead( BUTTON_1 ) )
      {
        t.fontsize -= 1;
        interval -= 1000;
        t.lcd_display( 10, 60, t.fontsize, 1000, String( interval ) );
        t.fill_screen( TFT_BLACK );
      }

      if ( ! digitalRead( BUTTON_2 ) )
      {
        t.fontsize += 1;
        interval += 1000;
        t.lcd_display( 10, 60, t.fontsize, 1000, String( interval ) );
        t.fill_screen( TFT_BLACK );
      }
  }
  
  if (PIR)
	  read_motion( "/pir", PIR );

  if (ENFORCER)
	  read_motion( "/enforcer", ENFORCER );

  if (DOOR1)
  {
      int door = 1;   // Door number
      int value = -1;
      char device[] = "/door0";
      int r = monitor_doors( device, door, DOOR1, value );
      if ( r )
          alert_door( device, door, value );
  }

  if (DOOR2)
  {
      int door = 2;   // Door number
      int value = -1;
      char device[] = "/door0";
      int r = monitor_doors( device, door, DOOR2, value );
      if ( r )
          alert_door( device, door, value );
  }

  if (LUX)
  {
    lux = luminosite();
    delta( "/lux", "LUX=", lux );
  }

  if ( SONIC )
  {
    sonic_value = get_TOF();
    delta( "/sonic", "Sonic=", sonic_value );
  }
	  
}

//  Read Boolean Motion
void read_motion( char* device, int PIN )
{

  if ( motion_count > 0 )
  {
    int lapse = millis() - last_time;
    if ( lapse > interval )
    {
      motion_count = 0;
      t.fill_screen( TFT_BLACK );
    }
  }

  motion = digitalRead( PIN );
  Serial.println( motion );
  if( motion )
  {
    if ( motion_count == 0 )
    {
        start_time = millis();
        t.fill_screen( TFT_BLACK );
    }
    last_time = millis();
    //    t.lcd_display( 1, 0, t.fontsize, 100, String( last_time ) );
    motion_count++;

    String mt = "Motion=" + String( motion_count );
    t.lcd_display( 1, 30, t.fontsize, 0, mt );

    if ( motion_count >= motion_threshold )
    {
        publish_message( device, String( motion_count ) );
    }
  }
}


//  Limitation only can read one range,
//  unless you have additional ld
//  Read Range of motion
void delta( char* device, String sensor_type, int d )
{
    String p = sensor_type + d;
    Serial.println( d );
    //  60 is row 2
    t.lcd_display( 1, 90, t.fontsize, 0, "           " );
    t.lcd_display( 1, 90, t.fontsize, 0, p );

      if ( d > 2 * ld || d < ld / 2)
      {
          ld = d;
          publish_message( device, String( d ) );
      }  
}


void alert_door( char* device, int i, int value )
{
    publish_message( device, String( value ) );
    String s = "Closed ";
    if ( value )
      s = "Open ";

    t.lcd_display( x_door + 10, y_door, s_door, 3000, s + String(i) );
    t.fill_screen( 0 );
    t.lcd_display( d1_x, d1_y, fsize, 0, String( digitalRead(DOOR1) ) );
    t.lcd_display( d2_x, d2_y, fsize, 0, String( digitalRead(DOOR2) ) );
}

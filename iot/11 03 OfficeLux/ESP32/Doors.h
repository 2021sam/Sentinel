//  doors.cpp
//  Wireless Door security system.
//  ESP32 Microcontroller code that responds with status of a door sensor.
//  Devices:
//    Panasonic Hall Effect Sensor
//
//  Sam Portillo
//  06/01/2020
//  09/25/2020  Live Version 2  - No polling, just notify when open.
//  04/27/2021  Remove LoRa for Bluetooth & Terminal capabilities
//  04/29/2021  Send 1 transmission when status changes:
//        Send 1 transmission when door opens.
//        Send 1 transmission when door closes.
//  05/01/2021  MQTT
//  05/26/2021  - TTGO T-Display
//  06/01/2021  Integrated door_monitor with class.


//  References:
//    http://reyax.com/products/rylr896/
//    https://us.amazon.com/Anmbest-KY-003-Effect-Magnetic-Arduino/dp/B07NWFX53H
//    https://www.youtube.com/watch?v=gnlDHDFDqQ8
//    https://www.youtube.com/watch?v=uozq47oxfwE


bool door_status[3];
String  door_status$ = "Closed";
void monitor_doors();
void alert_door(int, int);


//M5Stack:
int x_door = 20;
int y_door = 150;
int s_door = 6;
int d1_x  = 0;
int d1_y  = 200;
int d2_x  = 280;
int d2_y  = 200;
int fsize  = 4;


//TTGO:
//#define DOOR1 0
//#define DOOR2 35
//int x_door = 20;
//int y_door = 70;
//int s_door = 1;
//int d1_x  = 0;
//int d1_y  = 100;
//int d2_x  = 200;
//int d2_y  = 100;
//int fsize  = 1;


//  r = 0 → No change → Do not publish
//  r = 1 → Change    → Publish
int monitor_doors( char * device, int door, int PIN, int & value )
{
  value = digitalRead( PIN );
  if ( door_status[door] != value )
  {
    //door_status[DOOR1] = 1 - door_status[DOOR1];
    //door_status[DOOR1] = !door_status[DOOR1];
    door_status[door] = value;
    //    publish_topic_p[11] = (char) 48 + door;
    device[5] = (char) 48 + door;
    return 1;
  }
  
  return 0;
}

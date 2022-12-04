//  Time Of Flight
//  Arduino VL53L1X distance & PIR Motion Sensors
//  https://raw.githubusercontent.com/sparkfun/SparkFun_VL53L1X_Arduino_Library/master/examples/Example1_ReadDistance/Example1_ReadDistance.ino
//  https://www.l33t.uk/wp-content/uploads/2020/05/vl53l1X_Circuit.png
//  https://www.l33t.uk/ebay/vl53l1x/
#include <ComponentObject.h>
#include <RangeSensor.h>
#include <SparkFun_VL53L1X.h>
#include <vl53l1x_class.h>
#include <vl53l1_error_codes.h>
//#define BH1750a 0x23

SFEVL53L1X distanceSensor;

void setup_TOF()
{
  Wire.begin();
  if (distanceSensor.begin() != 0)          //Begin returns 0 on a good init
  {
    Serial.println("Sensor failed to begin. Please check wiring. Freezing...");
    while (1);
  }
  Serial.println("Sensor online!");
}


int get_TOF()
{
  distanceSensor.startRanging(); //Write configuration bytes to initiate measurement
  while (!distanceSensor.checkForDataReady())
  {
    delay(1);
  }
  int distance = distanceSensor.getDistance(); //Get the result of the measurement from the sensor
  distanceSensor.clearInterrupt();
  distanceSensor.stopRanging();

  float distanceInches = distance * 0.0393701;
  float distanceFeet = distanceInches / 12.0;

  return (int) distanceInches;
}







//void setup_sonic( int trigPin, int echoPin )
//{
//  pinMode(echoPin, INPUT);
//  pinMode(trigPin, OUTPUT);
//  digitalWrite( echoPin, HIGH );
//}
//
//
//int get_sonic( int echoPin, int trigPin )
//{
//  // Clear the trigPin by setting it LOW:
//  digitalWrite(trigPin, LOW);
//  //  delayMicroseconds(5);
//  delay( 10 );
//
//  // Trigger the sensor by setting the trigPin high for 10 microseconds:
//  digitalWrite(trigPin, HIGH);
//  //  delayMicroseconds(10);
//  delay( 10 );
//  digitalWrite(trigPin, LOW);
//  
//  // Read the echoPin. pulseIn() returns the duration (length of the pulse) in microseconds:
//  long duration = pulseIn(echoPin, HIGH, 100000);
//  
//  // Calculate the distance:
//  int distance = duration*0.034/2;
//  Serial.println( distance );
//  return distance;
//}

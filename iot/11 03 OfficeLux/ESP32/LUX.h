#include <Wire.h>
#define BH1750a 0x23

void setup_LUX()
{
  Wire.begin();
  Wire.setClock( 400000L );
  Wire.beginTransmission( BH1750a );
  Wire.write( 0x10 );
  Wire.endTransmission();
  delay( 1800 );
}


uint16_t luminosite(){
  uint16_t lux;
  Wire.requestFrom( BH1750a, 2 );
  while( Wire.available() )
  {
    lux = Wire.read();
    lux<<=8;
    lux = lux + Wire.read();
  }

  Wire.endTransmission();
  lux = lux / 1.2;
  return lux;
}
 

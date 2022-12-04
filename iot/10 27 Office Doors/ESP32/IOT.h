#include <WiFi.h>
#include <PubSubClient.h>
WiFiClient espClient;
PubSubClient client(espClient);


int location = 1;   //  1 = Blackhawk,  2 = BEAR
//String client_name      = "CourtYard";
//String client_name      = "Foyer";
String client_name    = "OfficeDoors";



class IOT
{
  public:
  int location;
  int device;
  String client_name;
  char ssid[20];
  char password[20];
  char mqtt_server[10];

  IOT( int location, String client_name ) : location( location ), client_name( client_name )
  {
      //    this->location = location;
      //    this->device = device;

      if ( location == 1 )
      {
          //const char* ssid          = "X50";
          //const char* password      = "996summerfun";
          strcpy( ssid, "Victorious" );
          strcpy( password, "2019summerfun" );
          strcpy( mqtt_server, "10.0.0.12" );
      }

      if ( location == 2 )
      {
          strcpy( ssid, "BEAR-2.4" );
          strcpy( password, "3393bear" );
          strcpy( mqtt_server, "10.1.10.183" );
      }
  }

  
};



IOT iot( location, client_name );


char client_name_char[30];
String subscription_sensor   = client_name + "/command";
String subscription_terminal = client_name + "/command/terminal";
void reconnect();
void concatenation( char* topic, char*, char* device );

String IpAddress2String(const IPAddress& ipAddress)
{
  return String(ipAddress[0]) + String(".") +\
  String(ipAddress[1]) + String(".") +\
  String(ipAddress[2]) + String(".") +\
  String(ipAddress[3])  ; 
}

void setup_WiFi()
{  
  t.fill_screen( TFT_RED );
  t.TFT_setTextColor( TFT_BLACK, TFT_RED );
  t.lcd_display( 30, 0, t.fontsize, 0, "No WiFi" );
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println( iot.ssid );
  
  int c = 0;
  while ( WiFi.status() != WL_CONNECTED )
  {
    t.lcd_display( 30, 50, t.fontsize, 0, String( c++ ) );
    WiFi.begin( iot.ssid, iot.password);         //  07/08/2021  WiFi crashed, may have been stuck in a loop.
    delay( 5000 );                      //  2000 is not enough but 3000 is.
    Serial.print(".");
    Serial.println( subscription_terminal );
  }
  
  if ( WiFi.status() == WL_CONNECTED )
  {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    IPAddress broadCast = WiFi.localIP();
    Serial.println( broadCast );
  
    t.TFT_setTextColor( TFT_WHITE, TFT_BLACK );
    t.fill_screen( TFT_GREEN );
    t.lcd_display( 60, 0, t.fontsize, 1000, "WiFi" );
    t.lcd_display( 0, 50, t.fontsize, 3000, IpAddress2String( broadCast ) );
  }
}




void publish_message( char* device, String s )
{ 
  char topic[ 50 ];
  client_name.toCharArray( client_name_char, client_name.length() + 1 );
  concatenation( topic, client_name_char, device );

  char message[ 50 ];
  //  s.toCharArray(message, s.length() );
  t.string_to_charArray( s, message );
  client.publish( topic, message );
}


void concatenation( char* a, char* b, char* c )
{
  while( *a++ = *b++ );
  a--;
  while( *a++ = *c++ );
}


void callback(char* topicc, byte* message_b, unsigned int length)
{
  String topic = String( topicc );
  Serial.print("Message arrived on topic: ");
  Serial.println(topic);
  Serial.print("Message: ");
  String message;

  for (int i = 0; i < length; i++)
  {
    Serial.print((char)message_b[i]);
    message += (char)message_b[i];
  }
  Serial.println();
  Serial.print( message );

  t.fill_screen( TFT_BLACK );
  t.lcd_display( 0, 50, 1, 0, topic );
  t.lcd_display( 0, 85, 1, 3000, message );
  t.fill_screen( TFT_BLACK );

  if ( topic.equals( subscription_terminal ) )
  {
      t.command( topic, message );
  }

  if ( topic.equals( subscription_sensor ) )
  {
        String c  = t.getNext( message, '=' );

        if ( c.equals("sonic") )
        {
            Serial.println( "***** Sonic = " + String( sonic_value ) );
            publish_message( "/sonic", String( sonic_value ) );
            delay( 1000 );
        }

        if ( c.equals("lux") )
        {
            publish_message( "/lux", String( lux ) );
        }

        if ( c.equals("shade") )
        {
            shade = message.toInt();
            //            shade = s.substring(6, s.length() ).toInt();
            String shv = "shade=" + String( shade );
            //            String shv = "set shade=";    // Works
            Serial.println( shv );
            t.lcd_display( 1, 80, t.fontsize, 5000, shv );
            publish_message( "/shade=", shv );
        }


        if ( c.equals("motion_threshold") )
        {
            motion_threshold = message.toInt();
            publish_message( "/motion_threshold", String( motion_threshold) );
        }
  }

}


void checkConnections()
{
  if ( WiFi.status() != WL_CONNECTED )
  {
    setup_WiFi();
  }

  if ( !client.connected() )
  {
    reconnect();
  }
  client.loop();
}


void reconnect()
{
    Serial.print("Attempting MQTT connection...");
    if (client.connect( client_name_char ))
    {
      Serial.println("connected");
      char subscription[30];
      subscription_terminal.toCharArray( subscription, 30 );
      client.subscribe( subscription );
      subscription_sensor.toCharArray( subscription, 30 );
      client.subscribe( subscription );
    } else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
    
    if (!client.connected())
    {
        t.fill_screen( TFT_RED );
        t.TFT_setTextColor( TFT_BLACK, TFT_RED );
        t.lcd_display( 30, 0, t.fontsize, 0, "No MQTT" );
        checkConnections();
    }

      t.TFT_setTextColor( TFT_WHITE, TFT_BLACK );
      t.lcd_display( 0, 100, t.fontsize, 3000, "MQTT" );
      t.lcd_display( 0, 100, t.fontsize, 2000, "Connection" );
      t.lcd_display( 0, 100, t.fontsize, 4000, client_name + "     "  );
      t.fill_screen( TFT_BLACK );
}

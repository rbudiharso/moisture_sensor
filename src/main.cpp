#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

int readSoil();
void setup_wifi();
void callback(char *topic, byte *payload, unsigned int length);
void reconnect();

/*  Soil Mositure Basic Example
    This sketch was written by SparkFun Electronics
    Joel Bartlett 
    August 31, 2015

    Basic skecth to print out soil moisture values to the Serial Monitor 

    Released under the MIT License(http://opensource.org/licenses/MIT)
*/

int val = 0;        //value for storing moisture value
int soilPin = A0;   //Declare a variable for the soil moisture sensor
int soilPower = D7; //Variable for Soil moisture Power

const char *ssid = "No Internet";
const char *password = "gargantuan";
const char *mqtt_server = "test.mosquitto.org";
const char *level_topic = "moisture_level";
const char *treshold_topic = "moisture_treshold";

WiFiClient espClient;
PubSubClient client(mqtt_server, 1883, espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;

void setup_wifi()
{

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++)
  {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect()
{
  // Loop until we're reconnected
  while (!client.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("wemosd1m1n1"))
    {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(level_topic, "Streaming soil moisture level");
      // ... and resubscribe
      client.subscribe(treshold_topic);
    }
    else
    {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//Rather than powering the sensor through the 3.3V or 5V pins,
//we'll use a digital pin to power the sensor. This will
//prevent corrosion of the sensor as it sits in the soil.

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT); // Initialize the BUILTIN_LED pin as an output
  pinMode(soilPower, OUTPUT);   //Set D7 as an OUTPUT
  digitalWrite(soilPower, LOW); //Set to LOW so no power is flowing through the sensor

  Serial.begin(115200); // open serial over USB
  setup_wifi();

  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

void loop()
{
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  //get soil moisture value from the function below and print it
  Serial.print("Soil Moisture = ");
  Serial.println(readSoil());

  snprintf(msg, MSG_BUFFER_SIZE, "Soil Moisture: %d", readSoil());
  Serial.print("Publish message: ");
  Serial.println(msg);
  client.publish(level_topic, msg);

  //This 1 second timefrme is used so you can test the sensor and see it change in real-time.
  //For in-plant applications, you will want to take readings much less frequently.
  delay(1000 * 60); //take a reading every second
}

//This is a function used to get the soil moisture content
int readSoil()
{
  digitalWrite(LED_BUILTIN, LOW);
  digitalWrite(soilPower, HIGH); //turn D7 "On"
  delay(10);                     //wait 10 milliseconds
  val = analogRead(soilPin);     //Read the SIG value form sensor
  digitalWrite(LED_BUILTIN, HIGH);
  digitalWrite(soilPower, LOW); //turn D7 "Off"
  return val;                   //send current moisture value
}

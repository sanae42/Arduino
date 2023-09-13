/*
* File: esp8266.ino
* Author: P Geng
* Refer to : https://blog.csdn.net/m0_52991090/article/details/122157194
* Date: 2023-09-13
* Description: Program for ESP8266 Board
* Version: 1.0
*/

#include <ESP8266WiFi.h>
#include <PubSubClient.h> // MQTT related library
#include <SoftwareSerial.h> // Soft serial port library

char P_NAME[] = "OnePlus";           //WIFI name
char P_PSWD[] = "1234567890";          //WIFI password
char sub[] = "TrashCanSub/1";    //Subscribed topic
char pub[] = "TrashCanPub";    //Published topic
int id = 1;
 
const char *ssid = P_NAME;
const char *password = P_PSWD;
const char *mqtt_server = "47.98.247.122"; //IP address of server
String reStr;
WiFiClient espClient;
PubSubClient client(espClient);

#define D5 (14)
#define D6 (12)

SoftwareSerial mqttSerial;     // Software Serial RX, TX
 
void setup_wifi()
{
  delay(10);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  randomSeed(micros());
}
 
void callback(char *topic, byte *payload, unsigned int length)
{
  for (int i = 0; i < length; i++)
  {
    mqttSerial.print((char)payload[i]);
  }
  mqttSerial.println();
}
 
void reconnect()
{
  while (!client.connected())
  {
    String clientId = "ESP8266Client";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) //Successfully connected
    {
      client.subscribe(sub);
    }
    else
    {
      delay(5000);
    }
  }
}
 
void setup()
{
  Serial.begin(9600);
  mqttSerial.begin(9600, SWSERIAL_8N1, D5, D6, false, 128);
  setup_wifi();
  client.setServer(mqtt_server, 1883); //1883 port
  client.setCallback(callback);
}
 
void loop()
{
 
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  if (mqttSerial.available() > 0)
  {
    reStr = mqttSerial.readStringUntil('\n');
    int str_len = reStr.length() + 1;
    char char_array[str_len];
    reStr.toCharArray(char_array, str_len);
    client.publish(pub, char_array);
  }
}

#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <SoftwareSerial.h> // 引入软串口库

char P_NAME[] = "OnePlus";           //设置热点名称
char P_PSWD[] = "1234567890";          //设置热点密码
char sub[] = "TrashCanSub/1";    //设置设备SUB名称
char pub[] = "TrashCanPub";    //设置设备PUB名称
int id = 1;
 
const char *ssid = P_NAME;
const char *password = P_PSWD;
const char *mqtt_server = "47.98.247.122";
String reStr;
WiFiClient espClient;
PubSubClient client(espClient);
// #define MSG_BUFFER_SIZE (50)
// char msg[MSG_BUFFER_SIZE];

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
    if (client.connect(clientId.c_str()))
    {
      // client.publish(pub, "{\"State\":\"OnLine\"}"); //连接成功后发消息：{"State":"OnLine"}
      client.subscribe(sub);
    }
    else
    {
      // Serial.print(client.state());
      delay(5000);
    }
  }
}
 
void setup()
{
  // pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(9600);
  mqttSerial.begin(9600, SWSERIAL_8N1, D5, D6, false, 128);
  setup_wifi();
  client.setServer(mqtt_server, 1883); //1883端口
  client.setCallback(callback);
  // digitalWrite(BUILTIN_LED, HIGH);
}
 
void loop()
{
 
  if (!client.connected())
  {
    reconnect();
  }
  client.loop();

  // delay(3000); //delay会造成字符错位
  if (mqttSerial.available() > 0)
  {
    reStr = mqttSerial.readStringUntil('\n');
    int str_len = reStr.length() + 1;
    char char_array[str_len];
    reStr.toCharArray(char_array, str_len);
    client.publish(pub, char_array);
  }
}

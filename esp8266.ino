#include <ESP8266WiFi.h>
#include <PubSubClient.h>
// #define BUILTIN_LED 2
char P_NAME[] = "OnePlus";           //设置热点名称
char P_PSWD[] = "1234567890";          //设置热点密码
char sub[] = "Sub/100052";    //设置设备SUB名称
char pub[] = "Pub/100052";    //设置设备PUB名称
 
const char *ssid = P_NAME;
const char *password = P_PSWD;
const char *mqtt_server = "47.98.247.122";
String reStr;
WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
char msg[MSG_BUFFER_SIZE];
int value = 0;
 
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
    Serial.print((char)payload[i]);
  }
  Serial.println();
}
 
void reconnect()
{
  while (!client.connected())
  {
    String clientId = "ESP8266Client";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str()))
    {
      client.publish(pub, "{\"State\":\"OnLine\"}"); //连接成功后发消息：{"State":"OnLine"}
      client.subscribe(sub);
    }
    else
    {
      Serial.print(client.state());
      delay(5000);
    }
  }
}
 
void setup()
{
  // pinMode(BUILTIN_LED, OUTPUT);
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
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

  delay(5000);
  char char_array[10] = "8266test";
  client.publish(pub, char_array);

  // if (Serial.available() > 0)
  // {
  //   reStr = Serial.readStringUntil('\n');
  //   //检测json数据是否完整
  //   int jsonBeginAt = reStr.indexOf("{");
  //   int jsonEndAt = reStr.lastIndexOf("}");
  //   if (jsonBeginAt != -1 && jsonEndAt != -1)
  //   {
  //     reStr = reStr.substring(jsonBeginAt, jsonEndAt + 1);
  //     int str_len = reStr.length() + 1;
  //     char char_array[str_len];
  //     reStr.toCharArray(char_array, str_len);
  //     client.publish(pub, char_array);
  //   }
  // }
}

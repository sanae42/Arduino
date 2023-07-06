#include "DHT.h" //DHT传感器库
#include <ArduinoJson.h>//导入JSON库，用来封装发送数据的格式

//HC-SR04 Ultrasonic Distance Sensor
#define PIN_TRIG 3
#define PIN_ECHO 2
//LED
#define PIN_LED 4
//DHT
#define PIN_DHT 5     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

StaticJsonDocument<500> sendJson;          // 创建JSON对象，用来存放发送数据
static int id = 1;

void setup() {
  Serial.begin(9600);
  //HC-SR04 Ultrasonic Distance Sensor
  pinMode(PIN_TRIG, OUTPUT);
  pinMode(PIN_ECHO, INPUT);
    //LED
  pinMode(PIN_LED, OUTPUT);
  //DHT
  dht.begin();
}

void loop() {
  //HC-SR04 Ultrasonic Distance Sensor
  // digitalWrite(trigPin, LOW); // Clears the trigPin condition
  // delayMicroseconds(2);
  digitalWrite(PIN_TRIG, HIGH);// Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duration; // duration of sound wave travel
  int distance; // distance measurement
  duration = pulseIn(PIN_ECHO, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration * 0.034 / 2; // Calculating the distance. Speed of sound wave divided by 2 (go and back)
  // Serial.print("Distance in CM: ");
  // Serial.print(distance );

  float h = dht.readHumidity();//读湿度
  float t = dht.readTemperature();//读温度(摄氏度)

  // Serial.print("Humidity:");
  // Serial.print(h);
  // Serial.print("% Temperature:");
  // Serial.print(t);
  // Serial.println("℃");

  // 将数据添加到JSON对象中，左边为标识符，右边为变量
  sendJson["Type"] = "TrashCan";
  sendJson["Id"] = id;
  sendJson["Distance"] = distance;
  sendJson["Humidity"] = h;
  sendJson["Temperature"] = t;
  //将对象转换成字符串，并向esp8266发送消息
  serializeJson(sendJson, Serial);  
  Serial.print("\n");

  // blink();

  delay(3000);
}

// LED blink function
// blink函数有点问题
void blink() {
  digitalWrite(PIN_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(500);                       
  digitalWrite(PIN_LED, LOW);    // turn the LED off by making the voltage LOW
  delay(500);                       
}

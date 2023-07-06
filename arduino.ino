#include "DHT.h" //DHT传感器库
#include <ArduinoJson.h>//导入JSON库，用来封装发送数据的格式
#include <SoftwareSerial.h>//库可以将Arduino的引脚，通过程序模拟成串口来使用

SoftwareSerial mqttSerial(2,3); //定义D2、D3分别为RX,tx

//HC-SR04 Ultrasonic Distance Sensor
#define PIN_TRIG 6
#define PIN_ECHO 5
//LED
#define PIN_LED 4
//DHT
#define PIN_DHT 7     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);

StaticJsonDocument<500> sendJson;          // 创建JSON对象，用来存放发送数据
StaticJsonDocument<500> readJson;          // 创建JSON对象，用来存放接收到的数据

static int id = 1;

void setup() {
  Serial.begin(9600);
  mqttSerial.begin(9600);
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
  serializeJson(sendJson, mqttSerial);  
  mqttSerial.print("\n");
  // serializeJson(sendJson, Serial);  
  // Serial.print("\n");
  
  // 无法读mqttSerial口，因此从8266读数据只能用Serial即0RX口读；写数据到mqttSerial口正常
  if(Serial.available() > 0){
    while (Serial.available() > 0) { // 判断串口缓冲区是否有消息
      String inputString = Serial.readStringUntil('\n');  
      // Serial.println(inputString);
      //检测json数据是否完整，若通过则进行下一步的处理
      // int jsonBeginAt = inputString.indexOf("{",1);
      // int jsonEndAt = inputString.lastIndexOf("}",inputString.lastIndexOf("}")-1);
      // if (jsonBeginAt != -1 && jsonEndAt != -1)
      {
        deserializeJson(readJson, inputString);                             //通过ArduinoJSON库将JSON字符串转换为方便操作的对象
        // 判断接收的指令
        if (readJson.containsKey("Distance"))   //判断是否包含标识符，如果是则进行下一步处理
        {
          int dis = (int)readJson["Distance"];
          Serial.print(dis);
          Serial.println();
        }
      }
    }
  }


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

#include "DHT.h" //DHT传感器库
#include <ArduinoJson.h>//导入JSON库，用来封装发送数据的格式
#include <SoftwareSerial.h>//库可以将Arduino的引脚，通过程序模拟成串口来使用
#include <U8g2lib.h>//OLED

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
//OLED
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);
// #define _SS_MAX_RX_BUFF  512  // 接收缓冲区
// #define _SS_MAX_TX_BUFF 512  // 发送缓冲区
// #define SERIAL_TX_BUFFER_SIZE 128
// #define SERIAL_RX_BUFFER_SIZE 128

//深度
#define Depth 100

/*
 项目使用 21996 字节（68%）的程序存储空间。最大值为 32256 字节。
 个全局变量使用 2113 个字节（103%）的动态内存，剩下 -65 个字节用于局部变量。最大值为 2048 字节。
内存不足；有关减少空间的提示，请参见 https://support.arduino.cc/hc/en-us/articles/360013825179。
数据部分超出开发板中的可用空间
Compilation error: 数据部分超出开发板中的可用空间
*/
StaticJsonDocument<128> sendJson;          // 创建JSON对象，用来存放发送数据
StaticJsonDocument<128> readJson;          // 创建JSON对象，用来存放接收到的数据
// DynamicJsonDocument sendJson(500);
// DynamicJsonDocument readJson(500);

static int id = 1;

//附近垃圾桶位置信息
const String nearestTrashCanLocation;
int mode = 1;

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
  //OLED
  u8g2.begin();

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
  // DynamicJsonDocument sendJson(500); // 创建JSON对象，用来存放发送数据
  // DynamicJsonDocument readJson(500); // 创建JSON对象，用来存放接收到的数据
  // 将数据添加到JSON对象中，左边为标识符，右边为变量
  sendJson["dataType"] = "trashCanDataCollect";
  sendJson["Id"] = id;
  sendJson["Distance"] = distance;
  sendJson["Humidity"] = h;
  sendJson["Temperature"] = t;

  //将对象转换成字符串，并向esp8266发送消息
  serializeJson(sendJson, mqttSerial);  
  mqttSerial.print("\n");
  // serializeJson(sendJson, Serial);  
  // Serial.print("\n");


  // 判断串口缓冲区是否有消息
  if(mqttSerial.available() > 0){
    if(mqttSerial.overflow()){
      String str = "software serial overflow\n";
      String box = mqttSerial.readStringUntil('\n');  
      Serial.println(str);
    }else{
      String inputString = mqttSerial.readStringUntil('\n');  
      // Serial.println(inputString);
      //检测json数据是否完整，若通过则进行下一步的处理
      // int jsonBeginAt = inputString.indexOf("{",1);
      // int jsonEndAt = inputString.lastIndexOf("}",inputString.lastIndexOf("}")-1);
      // if (jsonBeginAt != -1 && jsonEndAt != -1)
      // if(true)
      // {
        if(inputString == "normal"){

        }else if(inputString.compareTo("conservation") == 0){
            mode = 2;
        }else{
          nearestTrashCanLocation = inputString;
          // nearestTrashCanLocation += "\0";
        }



  // char json1[] ="{\"str\":\"welcome\",\"data1\":1351824120,\"data2\":[48.756080,2.302038],\"object\":{\"key1\":-254}}";
  // StaticJsonDocument<100> jsonBuffer;  
  // DeserializationError error = deserializeJson(jsonBuffer, json1);
  // if(error){
  //   Serial.println("error");
  // }else{
  //   Serial.println("no error");
  // }

        // deserializeJson(readJson, inputString);                             //通过ArduinoJSON库将JSON字符串转换为方便操作的对象
        // // 判断接收的指令
        // if (readJson.containsKey("dataType"))   //判断是否包含标识符，如果是则进行下一步处理
        // {
        //   String dataType = readJson["dataType"];
        //   if(dataType == "nearestTrashCanData"){
        //     // nearestTrashCanLocation = readJson["nearestTrashCanData"];
        //     if (readJson.containsKey("nearestTrashCanData")){
        //       String s = readJson["nearestTrashCanData"];
        //       Serial.println(s);
        //     }
        //   }
        // }




      // }
      // Serial.print(inputString+"\n");
    }
  }


  //OLED
  u8g2.setFont(u8g2_font_6x13_tf);  // 字体： https://github.com/olikraus/u8g2/wiki/fntlist12
  u8g2.setFontDirection(0);
  u8g2.firstPage();
  bool isFull = ((float)(Depth)-distance)/(float)(Depth)*100 > 90;
  do {
    if(isFull){
      // u8g2.clearBuffer();
      u8g2.setCursor(0, 9);
      u8g2.print("Full! ");
      u8g2.print(((float)(Depth)-distance)/(float)(Depth)*100);
      u8g2.print("\%");

      // nearestTrashCanLocation = "At the entrance of McDonald's. At the entrance of McDonald's. At the entrance of McDonald's";
      // u8g2.setCursor(0, 25);
      // u8g2.print(nearestTrashCanLocation);
      int line = 0;
      int characterInLine = 0;
      u8g2.setCursor(0, 25);
      for (char ch : nearestTrashCanLocation){
        if(ch == '\0') break;
        
        if(characterInLine == 20){
          line++;
          u8g2.setCursor(0, 25+line*10);
          characterInLine=0;
        }
        u8g2.print(ch);
        characterInLine++;
      }
    }else{
      // u8g2.clearBuffer();
      u8g2.setCursor(0, 9);
      u8g2.print("Not full:");
      u8g2.print(((float)(Depth)-distance)/(float)(Depth)*100);
      u8g2.print("\%");
    }

    // u8g2.setCursor(0, 47);
    // u8g2.drawStr(0, 47,"test 888888888888888888888888888888888888888888888888888888");
  } while ( u8g2.nextPage() );

  // blink();
  Serial.println(nearestTrashCanLocation);

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

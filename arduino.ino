/*
* File: arduino.ino
* Author: P Geng
* Date: 2023-09-13
* Description: Program for ELEGOO Development Board
* Version: 1.0
*/

#include "DHT.h" //DHT sensor library
#include <ArduinoJson.h> //JSON library
#include <U8g2lib.h> //OLED library

//HC-SR04 Ultrasonic Distance Sensor
#define PIN_TRIG 6
#define PIN_ECHO 5
//LED
#define PIN_LED 4
//DHT sensor
#define PIN_DHT 7     // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11
DHT dht(PIN_DHT, DHTTYPE);
//OLED
U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#define Depth 100 //depth of the trash can

StaticJsonDocument<128> sendJson;         
// StaticJsonDocument<128> readJson;        

static int id = 1; // id of the trash can

const String nearestTrashCanLocation; //Location information of nearby available trash can

int mode = 1; // rtrash can mode

int timeCount = 0; //Timing related variable
int previousDistance = 0; //Last measured distance

void setup() {
  Serial.begin(9600);

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
  digitalWrite(PIN_TRIG, HIGH);// Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  delayMicroseconds(10);
  digitalWrite(PIN_TRIG, LOW);
  long duration; // duration of sound wave travel
  int distance; // distance measurement
  duration = pulseIn(PIN_ECHO, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance = duration * 0.034 / 2; // Calculating the distance. Speed of sound wave divided by 2 (go and back)

  //DHT
  float h = dht.readHumidity();//read humidity
  float t = dht.readTemperature();//read temperature(℃)

  timeCount++;
  bool ifSendJSONData = false;
  bool bigChanged = ((float)(previousDistance)-distance)/(float)(Depth) > 0.1 || ((float)(previousDistance)-distance)/(float)(Depth) < -0.1;
	switch (mode)
	{
	case 1:
		ifSendJSONData = true;
		break;
	case 2:
		if(timeCount >= 1200 || bigChanged || t > 45){
      timeCount=0;
      previousDistance = distance;
      ifSendJSONData = true;
    }
		break;
  case 3:
		if(bigChanged || t > 45){
      previousDistance = distance;
      ifSendJSONData = true;
    }
		break;
	default:
		break;
	}

  if(ifSendJSONData){
    // put data into JSON object
    sendJson["dataType"] = "trashCanDataCollect";
    sendJson["Id"] = id;
    sendJson["Distance"] = distance;
    sendJson["Humidity"] = h;
    sendJson["Temperature"] = t;
    sendJson["Mode"] = mode;

    //Convert the JSON object into a string and send a message to esp8266
    serializeJson(sendJson, Serial);  
    Serial.print("\n");
  }

  //Determine if there are messages in the serial buffer
  if(Serial.available() > 0){
    {
      String inputString = Serial.readStringUntil('\n');  

        // Determine message type based on string length
        if(inputString.length()<3){
            mode = inputString.toInt();
        }else{
            nearestTrashCanLocation = inputString;
        }
    }
  }

  //OLED
  u8g2.setFont(u8g2_font_6x13_tf);  // font: https://github.com/olikraus/u8g2/wiki/fntlist12
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

      int line = 0;
      int characterInLine = 0;
      u8g2.setCursor(0, 25);
      String str = "the nearest trash can is "+nearestTrashCanLocation;
      for (char ch : str){
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
  } while ( u8g2.nextPage() );

  // judge if fire
  if(t>45){
    digitalWrite(PIN_LED, HIGH);   // turn the LED on (HIGH is the voltage level)
  }else{
    digitalWrite(PIN_LED, LOW);    // turn the LED off by making the voltage LOW
  }

  delay(3000);
}

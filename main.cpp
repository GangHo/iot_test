#include <Arduino.h>
#include <IO7F8266.h>
#include <SSD1306.h>
#include <string>
const int pulseA = 12;
const int pulseB = 13;
const int pushSW = 2;
volatile int lastEncoded = 0;
volatile long encoderValue = 0;
volatile long encoderValue2 = 0;
volatile bool pressed = false;

String user_html = "";
String number = "";
String person = "";
String menu = "";
String price = "";
String content = "";

int fried_price = 15000;
int spicy_price = 16000;
int soy_price = 16000;
int total_price = 0;
int num = 0;

int count = 0;
int s_count = 0;
String pub_flag = "OFF";
String staff_flag = "OFF";

unsigned long lastPublishMillis = -pubInterval;
SSD1306 display(0x3c, 4, 5, GEOMETRY_128_32);
char *ssid_pfix = (char *)"skh_button";

void message(char *topic, byte *payload, unsigned int payloadLength)
{
  byte2buff(msgBuffer, payload, payloadLength);
  StaticJsonDocument<512> root;
  DeserializationError error = deserializeJson(root, String(msgBuffer));

  if (error)
  {
    Serial.println("handleCommand: payload parse FAILED");
    return;
  }
}

void numberpush()
{
  if (encoderValue < 10)
  {
    number = "1";
    num = 1;
  }
  if (encoderValue > 9 && encoderValue < 20)
  {
    number = "2";
    num = 2;
  }
  if (encoderValue > 19 && encoderValue < 30)
  {
    number = "3";
    num = 3;
  }
  if (encoderValue > 29 && encoderValue < 40)
  {
    number = "4";
    num = 4;
  }
  if (encoderValue > 39 && encoderValue < 50)
  {
    number = "5";
    num = 5;
  }
  if (encoderValue > 49 && encoderValue < 60)
  {
    number = "6";
    num = 6;
  }
}
void person_numberpush()
{
  if (encoderValue < 10)
  {
    person = "1";
  }
  else if (encoderValue > 9 && encoderValue < 20)
  {
    person = "2";
  }
  else if (encoderValue > 19 && encoderValue < 30)
  {
    person = "3";
  }
  else if (encoderValue > 29 && encoderValue < 40)
  {
    person = "4";
  }
}
void contentpush()
{
  if (encoderValue < 10)
  {
    content = "Spoon ";
  }
  if (encoderValue > 9 && encoderValue < 20)
  {
    content = "Chopsticks";
  }
  if (encoderValue > 19 && encoderValue < 30)
  {
    content = "Water";
  }
  if (encoderValue > 29 && encoderValue < 40)
  {
    content = "Tissue";
  }
  if (encoderValue > 39 && encoderValue < 50)
  {
    content = "Staff Call";
  }
}

IRAM_ATTR void handleRotary()
{
  // Never put any long instruction
  int MSB = digitalRead(pulseA); // MSB = most significant bit
  int LSB = digitalRead(pulseB); // LSB = least significant bit

  int encoded = (MSB << 1) | LSB;         // converting the 2 pin value to single number
  int sum = (lastEncoded << 2) | encoded; // adding it to the previous encoded value
  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderValue++;

  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderValue--;

  lastEncoded = encoded; // store this value for next time
  if (encoderValue > 255)
  {
    encoderValue = 255;
  }
  else if (encoderValue < 0)
  {
    encoderValue = 0;
  }
}

IRAM_ATTR void buttonClicked() // 음식종류 선택
{
  pressed = true;
  Serial.println("pushed");
  count++;
  if (count == 3 || s_count == 2)
  {
    pub_flag = "ON";
    count = 0;
    s_count = 0;
  }
}
void publishData()
{
  StaticJsonDocument<512> root;
  JsonObject data = root.createNestedObject("d");
  if (staff_flag == "ON")
  {
    data["heartbeat"] = "alive";
    data["call"] = content;
    staff_flag = "OFF";
  }
  else
  {
    data["heartbeat"] = "alive";
    data["menu"] = menu;
    data["number"] = number;
    data["person"] = person;
    data["price"] = price;
  }
  serializeJson(root, msgBuffer);
  client.publish(evtTopic, msgBuffer);
}
void setup()
{
  Serial.begin(115200);
  pinMode(pushSW, INPUT_PULLUP);
  pinMode(pulseA, INPUT_PULLUP);
  pinMode(pulseB, INPUT_PULLUP);
  attachInterrupt(pushSW, buttonClicked, FALLING);
  attachInterrupt(pulseA, handleRotary, CHANGE);
  attachInterrupt(pulseB, handleRotary, CHANGE);

  initDevice();
  JsonObject meta = cfg["meta"];
  pubInterval = meta.containsKey("pubInterval") ? meta["pubInterval"] : 0;
  lastPublishMillis = -pubInterval;

  WiFi.mode(WIFI_STA);
  WiFi.begin((const char *)cfg["ssid"], (const char *)cfg["w_pw"]);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }
  // main setup
  Serial.printf("\nIP address : ");
  Serial.println(WiFi.localIP());

  client.setCallback(message);
  set_iot_server();
  iot_connect();

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  display.display();
}

void loop()
{
  delay(500);
  if (!client.connected())
  {
    iot_connect();
  }
  client.loop();
  if (pressed || ((pubInterval != 0) && (millis() - lastPublishMillis > pubInterval)))
  {
    // publishData();
    lastPublishMillis = millis();
  }
  Serial.println(encoderValue);
  if (encoderValue < 10)
  {
    display.clear();
    display.drawString(35, 0, "Fried chicken");
    display.display();
    if (pressed == true)
    {
      // std::string price = std::to_string(number); // int를 문자열로 변환 어케하지??
      pressed = false;
      encoderValue = 0;
      delay(1000);
      while (true)
      {
        delay(1000);
        Serial.println(encoderValue);
        numberpush();
        total_price = fried_price * num;
        price = String(total_price);
        display.clear();
        display.drawString(35, 0, "Fried chicken"); // 5000원
        display.drawString(0, 10, "Menu num: ");
        display.drawString(55, 10, number);
        display.drawString(30, 20, "Price:");
        display.drawString(60, 20, price);
        display.display();
        if (pressed == true)
        {

          pressed = false;
          encoderValue = 0;
          delay(1000);
          while (true)
          {
            person_numberpush();

            delay(1000);
            Serial.println(encoderValue);
            display.clear();
            display.drawString(35, 0, "Fried chicken"); // 15000원
            display.drawString(0, 10, "Menu num: ");
            display.drawString(75, 10, "person: ");
            display.drawString(55, 10, number);
            display.drawString(115, 10, person);
            display.drawString(30, 20, "Price:");
            display.drawString(60, 20, price);
            display.display();
            if (pressed == true)
            {
              menu = "fried chicken";
              pressed = false;
              encoderValue = 0;
              break;
            }
          }
          break;
        }
      }
      // publishData();
      pressed = false;
    }
  }
  if (encoderValue > 9 && encoderValue < 20)
  {
    display.clear();
    display.drawString(35, 0, "spicy chicken");
    display.display();
    if (pressed == true)
    {
      pressed = false;
      encoderValue = 0;
      delay(1000);
      while (true)
      {
        delay(1000);
        Serial.println(encoderValue);
        numberpush();
        total_price = spicy_price * num;
        price = String(total_price);
        display.clear();
        display.drawString(35, 0, "spicy chicken");
        display.drawString(0, 10, "Menu num: ");
        display.drawString(55, 10, number);
        display.drawString(30, 20, "Price:");
        display.drawString(60, 20, price);
        display.display();
        if (pressed == true)
        {
          pressed = false;
          encoderValue = 0;
          delay(1000);
          while (true)
          {
            person_numberpush();
            delay(1000);
            Serial.println(encoderValue);
            display.clear();
            display.drawString(35, 0, "spicy chicken");
            display.drawString(0, 10, "Menu num: ");
            display.drawString(75, 10, "person: ");
            display.drawString(55, 10, number);
            display.drawString(115, 10, person);
            display.drawString(30, 20, "Price:");
            display.drawString(60, 20, price);
            display.display();
            if (pressed == true)
            {
              menu = "spicy chicken";
              pressed = false;
              encoderValue = 0;
              break;
            }
          }
          break;
        }
      }
      // publishData();
      pressed = false;
    }
  }

  if (encoderValue > 19 && encoderValue < 30)
  {
    display.clear();
    display.drawString(35, 0, "soy sauce chicken");
    display.display();
    if (pressed == true)
    {
      pressed = false;
      encoderValue = 0;
      delay(1000);
      while (true)
      {
        delay(1000);
        Serial.println(encoderValue);
        numberpush();
        total_price = soy_price * num;
        price = String(total_price);
        display.clear();
        display.drawString(35, 0, "soy sauce chicken");
        display.drawString(0, 10, "Menu num: ");
        display.drawString(55, 10, number);
        display.drawString(30, 20, "Price:");
        display.drawString(60, 20, price);
        display.display();
        if (pressed == true)
        {
          pressed = false;
          encoderValue = 0;
          delay(1000);
          while (true)
          {
            person_numberpush();
            delay(1000);
            Serial.println(encoderValue);
            display.clear();
            display.drawString(35, 0, "soy sauce chicken");
            display.drawString(0, 10, "Menu num: ");
            display.drawString(75, 10, "person: ");
            display.drawString(55, 10, number);
            display.drawString(115, 10, person);
            display.drawString(30, 20, "Price:");
            display.drawString(60, 20, price);
            display.display();
            if (pressed == true)
            {
              menu = "soy sauce chicken";
              pressed = false;
              encoderValue = 0;
              break;
            }
          }
          break;
        }
      }
      // publishData();
      pressed = false;
    }
  }
  //--------------------------------------------스태프 호출-------------------------------------------
  if (encoderValue > 29 && encoderValue < 40)
  {
    display.clear();
    display.drawString(35, 0, "Staff call");
    display.display();
    if (pressed == true)
    {
      s_count++;
      delay(500);
      pressed = false;
      encoderValue = 0;
      delay(500);
      while (true)
      {
        delay(1000);
        Serial.println(encoderValue);
        contentpush();
        display.clear();
        display.drawString(35, 0, "Staff call");
        display.drawString(0, 10, "Content: ");
        display.drawString(50, 10, content);
        display.display();
        if (pressed == true)
        {
          s_count++;
          delay(500);
          pressed = false;
          delay(500);
          encoderValue = 0;
          break;
        }
      }
      staff_flag = "ON";
      delay(500);
      pressed = false;
    }
  }

  Serial.println(pub_flag);
  delay(500);
  if (pub_flag == "ON")
  {
    Serial.println("publish!");
    publishData();
    delay(300);
    pub_flag = "OFF";
  }
  delay(500);
}

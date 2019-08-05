#include <Servo.h>
#include <SoftwareSerial.h>

SoftwareSerial esp(10, 11);

const int triggerO = 2, echoO = 3, triggerH = 4, echoH = 5;
int distanceO, durationO, distanceH, durationH;

const int motorPin = 9;
const int motorDelay = 15;
int pos = 30;
Servo motor;

const String ssid = "Note4", pass = "4567890123";
const String host = "api.thingspeak.com", port = "80";
int timer, timegap;

bool firstTime = true;

void GetDistanceO()
{
  digitalWrite(triggerO,LOW);
  delayMicroseconds(2);
  digitalWrite(triggerO,HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerO,LOW);
  delayMicroseconds(2);
  durationO=pulseIn(echoO,HIGH);
  distanceO=durationO*0.034/2;
}

void GetDistanceH()
{
  digitalWrite(triggerH,LOW);
  delayMicroseconds(2);
  digitalWrite(triggerH,HIGH);
  delayMicroseconds(10);
  digitalWrite(triggerH,LOW);
  delayMicroseconds(2);
  durationH=pulseIn(echoH,HIGH);
  distanceH=durationH*0.034/2;
}

void GarbageLvl()
{
  GetDistanceH();
  int curtime = millis();
  if(distanceH <= 10 && distanceH >= 0)
  {
    Serial.println("FULL");
    if((curtime - timer) > 1800000 || firstTime)
    {
      String data = "GET https://api.thingspeak.com/update?api_key=P6F34LZKY5BT21D1&field1="+distanceH;
      SndCmd("AT+CIPSTART=\"TCP\",\""+host+"\","+port, "OK", 10);
      SndCmd("AT+CIPSEND="+String(data.length()+2), ">", 10);
      esp.println(data);
      timer = curtime;
    }
  }
  else if(distanceH <= 30 && distanceH >=10 || firstTime)
  {
    Serial.println("HALF");
    if((curtime - timer) > 1800000)
    {
      String data = "GET https://api.thingspeak.com/update?api_key=1YHHCR1GIFP0I4IU&field1="+distanceH;
      SndCmd("AT+CIPSTART=\"TCP\",\""+host+"\","+port, "OK", 10);
      SndCmd("AT+CIPSEND="+String(data.length()+2), ">", 10);
      esp.println(data);
      timer = curtime;
    }
  }
  else
  {
    Serial.println("EMPTY");
  }
  firstTime = false;
}

void LidOpen()
{
  while(pos <= 120)
  {
    motor.write(pos);
    pos += 5;
    delay(motorDelay);
  }
}

void LidClose()
{
  while(pos >= 30)
  {
    motor.write(pos);
    pos -= 5;
    delay(motorDelay);
  }
}

void BinState()
{
  GetDistanceO();
  if(distanceO >= 0)
  {
    if(distanceO <= 25)
    {
      LidOpen();
    }
    else
    {
      LidClose();
    }
    Serial.println(distanceO);
  }
}

void SndCmd(String cmd, char* reply, int tym)
{
  int ctym = 0;
  bool found = false;
  Serial.println(cmd);
  while(ctym < tym)
  {
    esp.println(cmd);
    if(esp.find(reply))
    {
      found = true;
      break;
    }
    ++ctym;
  }
  if(found) Serial.println("OK");
  else  Serial.println("FAIL");
}

void ConnectWifi()
{
  SndCmd("AT+CWMODE=1", "OK", 5);
  SndCmd("AT+CWJAP=\""+ssid+"\",\""+pass+"\"", "OK", 5);
  SndCmd("AT+CWMUX=0", "OK", 5);
}

void setup() {
  timer = millis();
  Serial.begin(9600);
  esp.begin(9600);
  pinMode(triggerO, OUTPUT);
  pinMode(echoO, INPUT);
  pinMode(triggerH, OUTPUT);
  pinMode(echoH, INPUT);
  motor.attach(motorPin);
  ConnectWifi();
  delay(2000);
}

void loop() {
  GarbageLvl();
  BinState();
  delay(1000);
}

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#include "nvmdata.h"
#include "dynamicdata.h"
#include "NTPClient.h"
#include "WebPage.h"

//network stuff
const char* ntpServer = "pool.ntp.org";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, 3600);
WebPage webPage;

// constants and pins
const int motor1 = 33;
const int motor2 = 32;
// timer stuff
const int delayTime_ms = 10;
const int SECONDS_1 = 1000 / delayTime_ms;
const int SECONDS_2 = 2000 / delayTime_ms;
const int SECONDS_5 = 5000 / delayTime_ms;
const int SECONDS_10 = 10000 / delayTime_ms;
const int SECONDS_100 = 100000 / delayTime_ms;
const int MILLISECONDS_50 = 50 / delayTime_ms;
const int MILLISECONDS_500 = 500 / delayTime_ms;

/// @brief 
void setupWiFi()
{
  String hostname = "twoweeldrive";
  if (NVMData::get().NetDataValid() == false)
  {
    DynamicData::get().setNewNetwork = true;
    Serial.println(WiFi.macAddress());
    WiFi.mode(WIFI_AP);
    WiFi.softAP(hostname.c_str(), "123Power");
    delay(1000);
    Serial.println(WiFi.softAPIP());
    DynamicData::get().ipaddress = WiFi.softAPIP().toString();
  }
  else
  {
    WiFi.mode(WIFI_STA);
    WiFi.setHostname(hostname.c_str());
    Serial.printf("SSID = %s\n", NVMData::get().GetNetName().c_str());
    WiFi.begin(NVMData::get().GetNetName().c_str(), NVMData::get().GetNetPassword().c_str());
    Serial.println(WiFi.macAddress());
    int maxWaitForNet = 0;
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(delayTime_ms);
      if (maxWaitForNet < SECONDS_1)
      {
        maxWaitForNet++;
      }
      else
      {
        Serial.print(".");
        WiFi.begin(NVMData::get().GetNetName().c_str(), NVMData::get().GetNetPassword().c_str());
        DynamicData::get().incErrorCounter("Wifi startup");
        maxWaitForNet = 0;
      }
    }
    Serial.println("");
    Serial.println("connected");
    Serial.println(WiFi.localIP());
    Serial.println(WiFi.dnsIP());
    DynamicData::get().ipaddress = WiFi.localIP().toString();
  }
}

// start one time
void setup() {
  Serial.begin(115200);
  Serial.println("Starting two weel drive...");
  pinMode(motor1, OUTPUT);
  pinMode(motor2, OUTPUT);
  analogWrite(motor1, 0);
  analogWrite(motor2, 0);
  NVMData::get().Init();
  Serial.println("NetName: "+  NVMData::get().GetNetName());
  setupWiFi();
  webPage.Init();
  DynamicData::get().Init();
}
void checkCommand() {
  int readP = DynamicData::get().readPointer;
  // check if new command there and if enabled
  if((readP != DynamicData::get().writePointer) && (DynamicData::get().timerOn)) {
    DynamicData::get().counter = DynamicData::get().driveData[readP].time;
    Serial.println("Timeset: " + String(DynamicData::get().counter));
    int leftPower = DynamicData::get().driveData[readP].leftpower;
    int rightPower = DynamicData::get().driveData[readP].rightpower;
    analogWrite(motor1, leftPower);
    analogWrite(motor2, rightPower);
    readP++;
    if (readP == DynamicData::numberOfDriveData) {
      readP = 0;
    }
    DynamicData::get().readPointer = readP;
  } else {
    analogWrite(motor1, 0);
    analogWrite(motor2, 0);
  }
}
// cyclic every 10ms
void loop() {
  delay(10);
  webPage.loop();
  if (DynamicData::get().counter > 0) {
    DynamicData::get().counter--;
  } else {
    checkCommand();
  }
}

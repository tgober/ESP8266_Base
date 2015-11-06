/*
 * Copyright (c) 2015, Majenko Technologies
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * * Neither the name of Majenko Technologies nor the names of its
 *   contributors may be used to endorse or promote products derived from
 *   this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/* Create a WiFi access point and provide a web server on it. */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>
#include "EPROMAnything.h"

#define DETECT_MATCH_PATTERN ((uint16_t)0xAAF0u)
#define WIFI_CONNECT_TIMEOUT_CNT ((uint16_t)50u)

static void readPinConfig(void);

const int ledPin = BUILTIN_LED;
const int buttonPin = 0;
const int adcOutPin = 2;
const char * ssidAP = "Cfg";
const char * pwd = "AutomatischesZeitalter";

static bool currentBtnStat = false;
static bool prevBtnStat = false;
static bool startAp = false;
static bool switchSetup = false;
static uint16_t curPwmOut = 0u;

struct
{
  uint16_t detectPattern;
  char ssid[100];
  char password[100];
  uint16_t prevValue;

} eepromContent;


ESP8266WebServer server(80);

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot()
{
  server.send(200, "text/html", "<h1>You are connected</h1><br/><a href='wlanSetup'>Setup WLAN</a>");
}

void handleWlanSetup()
{
  server.send(200, "text/html", "<h1>You are connected</h1><form method='POST' action='/wlanSetupConfirm'><a>SSID</a><input type='text' name='ssid' /><br /> <a>pwd</a><input type='text' name='password' /><br /> <input type='submit' value='Update' /></form>");
}

void handlePwdPost()
{
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  Serial.print("ssid = '" + ssid + "'");
  Serial.print("pwd = '" + password + "'");
  strcpy( eepromContent.ssid, ssid.c_str());
  strcpy( eepromContent.password, password.c_str());
  eepromContent.detectPattern = DETECT_MATCH_PATTERN;
  EEPROM_writeAnything(0, eepromContent);
  EEPROM.commit();
  server.send(200, "text/html", "OK - Data stored to EEPROM.");
}


void handlePostValue()
{
  String valStr = server.arg("value");
  Serial.print("new Value = " + valStr);
  curPwmOut = (uint16_t)valStr.toInt();
  server.send(200, "text/html", "OK - Value Set to" + String(curPwmOut));
}



void setupWifiApMode()
{
  Serial.println("Switching to AP mode");
  IPAddress Ip(192, 168, 3, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  WiFi.softAP(ssidAP, pwd);

  Serial.println("AP active");
  Serial.print("SSID: ");
  Serial.println(ssidAP);
  Serial.print("PWD: ");
  Serial.println(pwd);
  Serial.print("visit website under: ");
  Serial.println(Ip);
}

void setupWifiConnect()
{
  uint16_t timeout = 0u;
  Serial.println("try to connect to");
  Serial.print("SSID: ");
  Serial.println(eepromContent.ssid);
  WiFi.begin(eepromContent.ssid, eepromContent.password);

  while ((WiFi.status() != WL_CONNECTED) && timeout < WIFI_CONNECT_TIMEOUT_CNT)
  {
    delay(500);
    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() != WL_CONNECTED)
  {
    // FUUUU
    startAp = true;
    switchSetup = true;
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

void getEepromContent()
{
  EEPROM.begin(512);
  Serial.println();
  Serial.println("Startup. Loading EEPROM");
  EEPROM_readAnything(0, eepromContent);
  if (eepromContent.detectPattern == DETECT_MATCH_PATTERN)
  {
    curPwmOut = eepromContent.prevValue;
    startAp = false;
    Serial.println("Reading EEPROM done");
  }
  else
  {
    curPwmOut = 0;
    startAp = true;
    Serial.println("Reading EEPROM done but uninitialized");
  }
}

static void readPinConfig()
{
  static uint8_t btnDownCnt = 0;
  if (digitalRead(buttonPin))
  {
    if (btnDownCnt > 0)
    {
      // prevent overflow
      btnDownCnt--;
    }
  }
  else
  {
    if (btnDownCnt < 0xFF)
    {
      // prevent overflow
      btnDownCnt++;
    }
  }

  if ((btnDownCnt > 0x50) && !currentBtnStat)
  {
    currentBtnStat = true;
  }

  if ((btnDownCnt < 0x10) && currentBtnStat)
  {
    currentBtnStat = false;
  }

}


void setup() {
  pinMode(ledPin, OUTPUT);
  pinMode(adcOutPin, OUTPUT);
  pinMode(buttonPin, INPUT);

  delay(1000);
  Serial.begin(115200);

  getEepromContent();

  server.on("/", handleRoot);
  server.on("/wlanSetup", handleWlanSetup);
  server.on("/wlanSetupConfirm", HTTP_POST, handlePwdPost);
  server.on("/setValue", handlePostValue);
  server.begin();
  Serial.println("HTTP server started");
  switchSetup = true;
}

void loop()
{
  readPinConfig();
  server.handleClient();
  analogWrite(adcOutPin, curPwmOut);
  if (prevBtnStat == true && currentBtnStat == false)
  {
    switchSetup = true;
    startAp = !startAp;
  }
  prevBtnStat = currentBtnStat;


  if (switchSetup)
  {
    switchSetup = false;
    Serial.println("ok. Change. What now ...");


    if (startAp)
    {
      WiFi.disconnect();
      setupWifiApMode();
    }
    else
    {
      WiFi.softAPdisconnect(true);
      setupWifiConnect();
    }

  }
  else
  {
    digitalWrite(ledPin, LOW);
  }

  delay(1);

}






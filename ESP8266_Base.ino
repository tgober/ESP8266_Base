#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>
#include "ESP8266_Common.h"

static const uint8_t ledPin = BUILTIN_LED;
static const uint8_t buttonPin = 0;
static const uint8_t adcOutPin = 2;
static const char * ssidAP = "Cfg";
//static const char * pwd = "AutomatischesZeitalter";
static const char * pwd = "12341234";

char mdns_name[40] = "ESP8266";

static bool prevBtnStat = false;
static bool startAp = false;
static uint16_t curPwmOut = 0u;

// Public variables
ESP8266WebServer server(81);

void setup() 
{
  BTH_Init(buttonPin);
  pinMode(ledPin, OUTPUT);
  pinMode(adcOutPin, OUTPUT);

  delay(1000);
  Serial.begin(115200);

  Serial.println();
  Serial.println("Startup. Loading EEPROM");
  
  if (PER_getEepromContent())
  {
    curPwmOut = PER_getPwm();
    startAp = false;
    Serial.println("Reading EEPROM done, values found, not starting Cfg-AP");
  }
  else
  {
    curPwmOut = 0;
    startAp = true;
    Serial.println("Reading EEPROM done but uninitialized, starting Cfg-AP");
  }

  // Connect to WiFi, start Config-AP if not successful.
  // After timeout, ESP is reset to retry to connect
  setupWifiConnect(startAp);
  
  delay(200);
  
  startMDNS();
  registerUrls();
  server.begin();  
}

void loop()
{
  BTH_Step();
  server.handleClient();

  analogWrite(adcOutPin, curPwmOut);
  
  bool btnStat = GetBtnStat();
  if (prevBtnStat == true && btnStat == false)
  {
    startAp = true;
    digitalWrite(ledPin, HIGH);
    setupWifiConnect(true);
    startAp = false;
  }
  prevBtnStat = btnStat;

  digitalWrite(ledPin, LOW);
  delay(1);
}

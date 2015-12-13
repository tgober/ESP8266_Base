#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>


// DNS server
const static uint8_t DNS_PORT = 53;
static DNSServer DnsServer;


static const uint8_t ledPin = BUILTIN_LED;
static const uint8_t buttonPin = 0;
static const uint8_t adcOutPin = 2;
static const char * ssidAP = "Cfg";
//static const char * pwd = "AutomatischesZeitalter";
static const char * pwd = "12341234";

static bool prevBtnStat = false;
static bool startAp = false;
static bool switchSetup = false;
static uint16_t curPwmOut = 0u;



// Public variables
ESP8266WebServer server(80);



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
    Serial.println("Reading EEPROM done");
  }
  else
  {
    curPwmOut = 0;
    startAp = true;
    Serial.println("Reading EEPROM done but uninitialized");
  }
  
  registerUrl();
  server.begin();

  Serial.println("HTTP server started");
  switchSetup = true;
}

void loop()
{
  BTH_Step();
  if (startAp)
  {
    DnsServer.processNextRequest();
  }
  server.handleClient();

  analogWrite(adcOutPin, curPwmOut);
  bool btnStat = GetBtnStat();
  if (prevBtnStat == true && btnStat == false)
  {
    switchSetup = true;
    startAp = !startAp;
  }
  prevBtnStat = btnStat;


  if (switchSetup)
  {
    switchSetup = false;
    Serial.print("ok. Change. Swtich to ");

    WiFi.disconnect();
    if (startAp)
    {
      Serial.println("AP mode");
      setupWifiApMode();
    }
    else
    {
      Serial.println("WiFi mode");
      setupWifiConnect();
    }


  }
  else
  {
    digitalWrite(ledPin, LOW);
  }

  delay(1);

}

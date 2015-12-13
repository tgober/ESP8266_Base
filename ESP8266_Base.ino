

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <EEPROM.h>
#include "EPROMAnything.h"

#define DETECT_MATCH_PATTERN ((uint16_t)0xAAF0u)
#define WIFI_CONNECT_TIMEOUT_CNT ((uint16_t)50u)

// DNS server
const static uint8_t DNS_PORT = 53;
static DNSServer DnsServer;


static void readPinConfig(void);

static const uint8_t ledPin = BUILTIN_LED;
static const uint8_t buttonPin = 0;
static const uint8_t adcOutPin = 2;
static const char * ssidAP = "Cfg";
//static const char * pwd = "AutomatischesZeitalter";
static const char * pwd = "12341234";

static bool currentBtnStat = false;
static bool prevBtnStat = false;
static bool startAp = false;
static bool switchSetup = false;
static uint16_t curPwmOut = 0u;

static struct
{
  uint16_t detectPattern;
  char ssid[100];
  char password[100];
  uint16_t prevValue;
} eepromContent;



// Public variables
ESP8266WebServer server(80);



static void setupWifiApMode()
{
  Serial.println("Switching to AP mode");
  IPAddress Ip(192, 168, 3, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(Ip, Ip, NMask);
  WiFi.softAP(ssidAP, pwd);
  delay(500); // Without delay I've seen the IP address blank
  
  DnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  DnsServer.start(DNS_PORT, "*", Ip);

  Serial.println("AP active");
  Serial.print("SSID: ");
  Serial.println(ssidAP);
  Serial.print("PWD: ");
  Serial.println(pwd);
  Serial.print("visit website under: ");
  Serial.println(WiFi.softAPIP());

  WiFi.printDiag(Serial);
}

static void setupWifiConnect()
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
  Serial.println();

  if (WiFi.status() != WL_CONNECTED)
  {
    // FUUUU
    startAp = true;
    switchSetup = true;
  }
  else
  {
    Serial.print("connected. IP is: ");
    Serial.println(WiFi.localIP());
  }

  
  String id_prefix = "OPENHAB_TESTLUDER_";
  String id = id_prefix + ESP.getFlashChipId();
  if (MDNS.begin(id.c_str())) 
  {
    Serial.print("mDNS responder started: ");
    Serial.println(id);
    MDNS.addService("http", "tcp", 80);
  }
  else
  {
    Serial.println("Error setting up MDNS responder!");
  }

  Serial.println("");
  Serial.println("WiFi connected");
}

static void getEepromContent()
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

  registerUrl();
  
  Serial.println("HTTP server started");
  switchSetup = true;
}

void loop()
{
  readPinConfig();
  if (startAp)
  {
    DnsServer.processNextRequest();
  }
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
    server.begin();

  }
  else
  {
    digitalWrite(ledPin, LOW);
  }

  delay(1);

}

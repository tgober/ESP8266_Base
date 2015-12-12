

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

const uint8_t ledPin = BUILTIN_LED;
const uint8_t buttonPin = 0;
const uint8_t adcOutPin = 2;
const char * ssidAP = "Cfg";
//const char * pwd = "AutomatischesZeitalter";
const char * pwd = "12341234";

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
  server.send(200, "text/html", "<!doctype html><body><head></head><html><h1>You are connected</h1><br><a href=\"wlanSetup\">Setup WLAN</a></body></html>");
}

void handleWlanSetup()
{
  server.send(200, "text/html", "<!doctype html><body><head></head><html><h1>You are connected</h1><form method=\"POST\" action=\"/wlanSetupConfirm\"><label id=\"ssid\">SSID</label><input id=\"ssid\" name=\"ssid\"><br><label id=\"pwd\">pwd</label><input id=\"pwd\" name=\"password\"><br><input type=\"submit\" value=\"Update\"></form></body></html>");
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
  server.send(200, "text/html", "<!doctype html><body><head></head><html>OK - Data stored to EEPROM.</body></html>");
}


void handlePostValue()
{
  String valStr = server.arg("value");
  Serial.print("new Value = " + valStr);
  curPwmOut = (uint16_t)valStr.toInt();
  server.send(200, "application/json", "{\"status\":\"Value Set to" + String(curPwmOut) + "\",\"value\":"+curPwmOut+"}");
}



void setupWifiApMode()
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

  

  if (MDNS.begin("OPENHAB_TESTLUDER")) 
  {
    Serial.println("mDNS responder started");
    MDNS.addService("http", "tcp", 80);
  }
  else
  {
    Serial.println("Error setting up MDNS responder!");
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
    Serial.println("ok. Change. What now ...");

    WiFi.disconnect();
    if (startAp)
    {
      //WiFi.disconnect();
      //WiFi.mode(WIFI_AP);
      setupWifiApMode();
    }
    else
    {
      //WiFi.softAPdisconnect(true);
      //WiFi.mode(WIFI_STA);
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

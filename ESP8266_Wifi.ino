#include <ESP8266mDNS.h>
#include "ESP8266_Common.h"

#define WIFI_CONNECT_TIMEOUT_CNT ((uint16_t)50u)



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
  Serial.println(PER_getSSID());
  WiFi.begin(PER_getSSID(), PER_getPassword());

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

int scanWiFis()
{
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1) {
    Serial.println("Couldn't scan for wifis");
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);

  return numSsid;
}


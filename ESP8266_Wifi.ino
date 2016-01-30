#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include "ESP8266_Common.h"

/* Unused at the moment
// This is set by WiFiManager Callback to trigger saving of extra parameters
bool shouldSaveConfig = false;

void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}
*/

void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.print("Entered config mode with SSID ");
  //if you used auto generated SSID, print it
  Serial.print(myWiFiManager->getConfigPortalSSID());
  Serial.print(" and IP ");
  Serial.println(WiFi.softAPIP());
}

void setupWifiConnect(bool forceStartAp = false)
{
  WiFiManager wifiManager;

  //reset saved settings - for debugging only
  //wifiManager.resetSettings();

  WiFiManagerParameter custom_mdns_name("mdns_name", "mDNS Name", mdns_name, 40);
  wifiManager.addParameter(&custom_mdns_name);

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);

  // set callback for user-defined setings to be saved
  //wifiManager.setSaveConfigCallback(saveConfigCallback);

  //sets timeout until configuration portal gets turned off
  //useful to make it all retry or go to sleep
  //in seconds
  wifiManager.setTimeout(300);

  //wifiManager.setDebugOutput(false);

  bool managerResult;
  
  if (forceStartAp == true) {
    Serial.println("Forcibly start Config-AP with WiFiManager...");
    managerResult = wifiManager.startConfigPortal(ssidAP);
  } else {
    managerResult = wifiManager.autoConnect(ssidAP);
  }

  //fetches ssid and pass and tries to connect
  //if it does not connect it starts an access point with the specified name
  //and goes into a blocking loop awaiting configuration
  if(!managerResult) {
    Serial.println("failed to connect and hit timeout");
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(1000);
  }

  //if you get here you have connected to the WiFi

  strcpy(mdns_name, custom_mdns_name.getValue());
    
}

void startMDNS()
{
  if (MDNS.begin(mdns_name))
  {
    Serial.print("mDNS responder started: ");
    Serial.println(mdns_name);
    
    MDNS.addService("http", "tcp", 80);
    
    MDNS.addService("status", "tcp", 80);
    MDNS.addServiceTxt("status", "tcp", "path", "/status");
  }
  else
  {
    Serial.println("Error setting up MDNS responder!");
  }

}


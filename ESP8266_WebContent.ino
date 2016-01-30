#include "ESP8266_Common.h"

const char HTML_PREFIX[] PROGMEM = "<!doctype html><html><head><style type='text/css'>body{font:sans-serif;}</style><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"></head>";
const char HTML_SUFFIX[] PROGMEM = "</body></html>";

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot()
{
  String response;
  response += FPSTR(HTML_PREFIX);
  response += "<h1>You are connected to " + String(mdns_name) + "</h1><br>";
  response += "<a>Value: " + String(curPwmOut) + "</a>";
  response += FPSTR(HTML_SUFFIX);
  server.send(200, "text/html",  response);
}

void sendStatus()
{
  String json = "{";
  json += "\"value\":" + String(curPwmOut) + ",";
  json += "\"min\":0,\"max\":" + String(PWMRANGE) + ",";
  json += "\"wifi\":{";
  json +=   "\"ssid\":\"" + WiFi.SSID() + "\",";
  json +=   "\"rssi\":" + String(WiFi.RSSI()) +",";
  json +=   "\"mac\":\"" + WiFi.macAddress() + "\"";
  json += "}}";
      
  server.send(200, "application/json", json);
}

void setNewValue()
{
  String valStr = server.arg("value");
  Serial.print("new Value = " + valStr);
  uint16_t newPwmOut = (uint16_t)valStr.toInt();
  if (newPwmOut != curPwmOut)
  {
    if (newPwmOut >= PWMRANGE)
    {
      server.send(400, "application/json", "{\"message\":\"Value too big\",\"value\":"+String(newPwmOut)+"}");
      return;
    }
    curPwmOut = newPwmOut;
  }
  
  if(curPwmOut != PER_getPwm())
  {
    PER_setPwm(curPwmOut);
    PER_saveContent();
  }
  
  server.send(200, "application/json", "{\"message\":\"Value set\",\"value\":"+String(curPwmOut)+"}");
}

void handleStatus() 
{
  if (server.method() == HTTP_POST) {
    setNewValue();
  } else {
    sendStatus();
  }
}


void registerUrls()
{
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.onNotFound(handleRoot);
}


#include "ESP8266_Common.h"

String HTML_PREFIX = "<!doctype html><html><head><meta charset=\"utf-8\"><meta name=\"viewport\" content=\"width=device-width,initial-scale=1\"></head>";
String HTML_SUFFIX = "</body></html>";

/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot()
{
  server.send(200, "text/html", HTML_PREFIX + "<h1>You are connected to ESP8266</h1><br><a href=\"wlanSetup\">Setup WLAN</a><br><a>Value: " + String(curPwmOut) + "</a>" + HTML_SUFFIX);
}

void handleWlanSetup()
{
  int numSsid = scanWiFis();
  String html = HTML_PREFIX + "<h1>Edit WiFi credentials</h1>" +
                  "<form method=\"POST\" action=\"/wlanSetup\">" +
                  "<label for=\"ssid\">SSID</label>" +
                  "<br>" +
                  "<select name=\"ssid\">";

  // print the network number and name for each network found:
    for (int thisNet = 0; thisNet < numSsid; thisNet++) {
      html += "<option value=\"";
      html += WiFi.SSID(thisNet);
      html += "\">";
      html += WiFi.SSID(thisNet);
      html += "(";
      html += WiFi.RSSI(thisNet);
      html += " dBm)</option>";
     }

  html += "</select>";
  html += "<br><label for=\"pwd\">pwd</label>";
  html += "<br><input id=\"pwd\" name=\"password\" type=\"password\" required>";
  html += "<br><input type=\"submit\" value=\"Update\">";
  html += "</form>";
  html += HTML_SUFFIX;

  server.send(200, "text/html", html);
}

void handlePwdPost()
{
  String ssid = server.arg("ssid");
  String password = server.arg("password");
  Serial.print("ssid = '" + ssid + "'");
  Serial.print("pwd = '" + password + "'");
  PER_setSSID(ssid.c_str());
  PER_setPassword(password.c_str());
  PER_setPersistanceContentValid();
  PER_saveContent();
  server.send(200, "text/html", HTML_PREFIX + "OK - Data stored to EEPROM." + HTML_SUFFIX);
}


void handlePostValue()
{
  String valStr = server.arg("value");
  Serial.print("new Value = " + valStr);
  curPwmOut = (uint16_t)valStr.toInt();
  if(curPwmOut != PER_getPwm())
  {
    PER_setPwm(curPwmOut);
    PER_saveContent();
  }
  server.send(200, "application/json", "{\"status\":\"Value Set to" + String(curPwmOut) + "\",\"value\":"+curPwmOut+"}");
}

void registerUrl()
{
  server.on("/", handleRoot);
  server.on("/wlanSetup", HTTP_GET, handleWlanSetup);
  server.on("/wlanSetup", HTTP_POST, handlePwdPost);
  server.on("/setValue", handlePostValue);
}


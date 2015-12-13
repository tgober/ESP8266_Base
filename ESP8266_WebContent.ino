
void registerUrl()
{
  server.on("/", handleRoot);
  server.on("/wlanSetup", HTTP_GET, handleWlanSetup);
  server.on("/wlanSetup", HTTP_POST, handlePwdPost);
  server.on("/setValue", handlePostValue);
}


/* Just a little test message.  Go to http://192.168.4.1 in a web browser
 * connected to this access point to see it.
 */
void handleRoot()
{
  server.send(200, "text/html", "<!doctype html><body><head></head><html><h1>You are connected</h1><br/><a href=\"wlanSetup\">Setup WLAN</a><br/><a>Value: " + String(curPwmOut) + "</a></body></html>");
}

void handleWlanSetup()
{
  server.send(200, "text/html", "<!doctype html><body><head></head><html><h1>You are connected</h1><form method=\"POST\" action=\"/wlanSetup\"><label id=\"ssid\">SSID</label><input id=\"ssid\" name=\"ssid\"><br><label id=\"pwd\">pwd</label><input id=\"pwd\" name=\"password\"><br><input type=\"submit\" value=\"Update\"></form></body></html>");
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



#ifndef ESP8266_COMMON__H
#define ESP8266_COMMON__H

void BTH_Init(uint8_t button);
void BTH_Step(void);
bool GetBtnStat(void);
bool PER_getEepromContent(void);
void PER_saveContent(void);
void PER_setPersistanceContentValid(void);
const char * PER_getSSID(void);
void PER_setSSID(const char * ssid);
const char * PER_getPassword(void);
void PER_setPassword(const char * pwd);
uint16_t PER_getPwm(void);
void PER_setPwm(uint16_t value);
void registerUrl();
void setupWifiApMode();
void setupWifiConnect();

#endif

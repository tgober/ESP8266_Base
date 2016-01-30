
#ifndef ESP8266_COMMON__H
#define ESP8266_COMMON__H

void BTH_Init(uint8_t button);
void BTH_Step(void);
bool GetBtnStat(void);
bool PER_getEepromContent(void);
void PER_saveContent(void);
void PER_setPersistanceContentValid(void);
uint16_t PER_getPwm(void);
void PER_setPwm(uint16_t value);
void registerUrls();
void setupWifiConnect(bool forceStartAp);
void startMDNS();

#endif

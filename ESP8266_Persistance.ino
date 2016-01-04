#include <EEPROM.h>
#include "EPROMAnything.h"
#include "ESP8266_Common.h"

#define DETECT_MATCH_PATTERN ((uint16_t)0xAAF0u)

typedef struct
{
  uint16_t detectPattern;
  char ssid[100];
  char password[100];
  uint16_t prevValue;
} PER_eepromContent_t;

static PER_eepromContent_t PER_Content;

bool PER_getEepromContent(void)
{
  EEPROM.begin(512);
  EEPROM_readAnything(0, PER_Content);
  return (PER_Content.detectPattern == DETECT_MATCH_PATTERN);
}

void PER_saveContent(void)
{
  EEPROM_writeAnything(0, PER_Content);
  EEPROM.commit();
}

void PER_setPersistanceContentValid(void)
{
  PER_Content.detectPattern = DETECT_MATCH_PATTERN;
}

const char * PER_getSSID(void)
{
  // TODO: create copy
  return PER_Content.ssid;
}

void PER_setSSID(const char * ssid)
{
  // TODO: add size or use STRING
  strcpy(PER_Content.ssid, ssid);
}

const char * PER_getPassword(void)
{
  // TODO: create copy
  return PER_Content.password;
}

void PER_setPassword(const char * pwd)
{
  // TODO: add size or use STRING
  strcpy(PER_Content.password, pwd);
}

uint16_t PER_getPwm(void)
{
  return PER_Content.prevValue;
}

void PER_setPwm(uint16_t value)
{
  PER_Content.prevValue = value;
}

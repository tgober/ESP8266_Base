#include "ESP8266_Common.h"

static uint8_t buttonIo = 0xFF;
static uint8_t btnDownCnt = 0;

static bool currentBtnStat = false;


void BTH_Init(uint8_t button)
{
  pinMode(buttonIo, INPUT);
  buttonIo = button;
}

void BTH_Step(void)
{
  if(buttonIo == 0xFF)
  {
    return;
  }
  
  if (digitalRead(buttonIo))
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

bool GetBtnStat(void)
{
  return currentBtnStat;
}


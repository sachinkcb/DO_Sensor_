/* 
  PM_APP.cpp - Application file is used to read the registor values of energy meter ic , calculate the suggested/ required capbank value and also used for detection of 
               power source
            
  Dev: Infiplus Team 
  May 2021  
*/

#include "CPondConfig.h"
#include "stdio.h"
#include <ArduinoJson.h>

#define SERIAL_DEBUG
#ifdef SERIAL_DEBUG
#define debugPrint(...) Serial.print(__VA_ARGS__)
#define debugPrintln(...) Serial.println(__VA_ARGS__)
#define debugPrintf(...) Serial.printf(__VA_ARGS__)
#define debugPrintlnf(...) Serial.println(F(__VA_ARGS__))
#else
#define debugPrint(...)    //blank line
#define debugPrintln(...)  //blank line
#define debugPrintf(...)   //blank line
#define debugPrintlnf(...) //blank line
#endif

/***************************************************************************
*  Safe copy Function to avoid overflow 
****************************************************************************/
static void safeStrcpy(char * destination, const char * source, int sizeofDest)
{
  strncpy(destination,source,sizeofDest-1);
  destination[sizeofDest-1] = 0;
}

/* Construct */
CPondConfig::CPondConfig(){}
/* Destruct */
CPondConfig::~CPondConfig(){}
/************************************************************
 * Load Pond Setting From File in local Veriables
*************************************************************/
int CPondConfig::loadPondConfig(FILESYSTEM* filesystem)
{
  char rdata[1024];
  int ret = 0;
  int Size = filesystem->getFileSize(FNAME_PONDSETTING);
  if (Size >= 1024)
  { 
    debugPrintln("Wrong Setting file size...");
    return 0;
  }
  /*Read File in data streM*/
  ret = filesystem->readFile(FNAME_PONDSETTING,rdata);
  if(ret > 0)
  {
    DynamicJsonDocument PondInfo(1024);    
    /*De serilaize the file json*/
    DeserializationError err = deserializeJson(PondInfo, rdata);
    if (err.code() == DeserializationError::Ok)
    {
      /*Read Pond Setting Version*/
      if(PondInfo.containsKey("PondSettingVer"))
      {
          m_dPondSettingVer = PondInfo["PondSettingVer"];
      }
      else
      {
          m_dPondSettingVer = 0;
      }
      /*Read Pond Number*/
      // if(PondInfo.containsKey("PondNum"))
      // {
      //     m_u8PondNum = PondInfo["PondNum"];
      // }
      // else
      // {
      //     m_u8PondNum = 0;
      // }
      // /*Read sampleRate*/
      // if(mealInfo.containsKey("sampleRate"))
      // {
      //     m_u8SampleRate = mealInfo["sampleRate"];
      // }
      // else
      // {
      //     m_u8SampleRate = 0;
      // }
      /*Read Pond wise Pond data*/
      for (uint8_t pnum = 0; pnum < TOTAL_PONDS; pnum++)
      {
        bool dataMissing = false;
        if(PondInfo["PondSettings"][pnum].containsKey("name"))
        {
          safeStrcpy(m_oPondSettingList[pnum].m_cName, (const char *)PondInfo["PondSettings"][pnum]["name"], 10);
        }
        else
        {
          dataMissing = true;
        }
         /*read pid for each pond*/
        if(PondInfo["PondSettings"][pnum].containsKey("pid"))
        {
          //m_oPondSettingList[pnum].m_cPid = PondInfo["PondSettings"][pnum]["pid"];
          safeStrcpy(m_oPondSettingList[pnum].m_cPid, (const char *)PondInfo["PondSettings"][pnum]["pid"], 20);
        }
        else
        {
          dataMissing = true;
        }
        
        /*Read wt*/
        if(PondInfo["PondSettings"][pnum].containsKey("Id"))
        {
          m_oPondSettingList[pnum].m_u64PondId = PondInfo["PondSettings"][pnum]["Id"];
        }
        else
        {
          dataMissing = true;
        }
        /* If data misssing set default*/
        if(dataMissing)        
        {
          safeStrcpy(m_oPondSettingList[pnum].m_cName, "P0", 10);
          m_oPondSettingList[pnum].m_u64PondId = 0;
         // m_oPondSettingList[pnum].m_u8NumofSamples = 3;
        }
      }
    }
  }
  /*Clear last meal data*/
 // clearPondMealData();
  /*Print Setting*/
  debugPrintln(m_dPondSettingVer);
  //debugPrintln(m_u8PondNum);
  for(int i = 0; i < 10; i++)
  {
    debugPrint(m_oPondSettingList[i].m_cName);debugPrint(" : ");
    debugPrint(m_oPondSettingList[i].m_u64PondId); debugPrint(" : "); 
    debugPrint(m_oPondSettingList[i].m_cPid); debugPrint(" : "); 
   // debugPrintln(m_oMealSettingList[i].m_u8PondId);
  }
  return 1;
}

/************************************************************
 * Read Pond Setting From File
*************************************************************/
int CPondConfig::readSetting(FILESYSTEM* filesystem,char *SettingData)
{
  char rdata[1024];
  int ret = 0;
  int Size = filesystem->getFileSize(FNAME_PONDSETTING);
  debugPrintln(Size);
  if (Size >= 1024)
  { 
    debugPrintln("Wrong Setting file size...");
    return 0;
  }
  /*Read File in data streM*/
  ret = filesystem->readFile(FNAME_PONDSETTING,SettingData);
  debugPrintln(SettingData);
  if(ret > 0)
  {
    return 1;
  }
  return 0;
}

/************************************************************
 * Write Pond setting to File
*************************************************************/
int CPondConfig::writeSetting(FILESYSTEM* filesystem)
{

}

  
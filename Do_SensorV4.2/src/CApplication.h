#ifndef APP_H
#define APP_H

#define TOSTR(x) #x
#define STRINGIFY(x) TOSTR(x)

#define FW_VERSION 3
#define MDASH_APP_NAME "NA_DO_V" STRINGIFY(FW_VERSION) 

#include <ArduinoJson.h>
#include <Ticker.h>
#include "CAT24C32.h"
#include "BSP.h"
#include "CBackupStorage.h"
#include "CDO_Sensor.h"
#include "CRfid.h"
#include "CHttp.h"
#include "DISPLAY.h"

#define NO_FRAME    0
#define TOUT_FRAME  1
#define VDIFF_FRAME 2
#define ACK_FRAME   3
#define CALL_FRAME  4
#define FAULT_FRAME 5
#define LIVE_FRAME      0
#define HISTORY_FRAME   1
#define NOFLT 0

#define LINE_FTLIND_TIMOUT 600  // 10 min
#define RLY_ON  HIGH
#define RLY_OFF  LOW

class cApplication
{
    private:        

        uint8_t m_u8DisplayConter, m_u8SendBackUpFrameConter;
        int m_iTimeOutFrameConter;
        int m_iRtcSyncCounter;
		int m_iTimerDisplayChange, m_iHooterCounter,m_iGeneratorFaultCounter,m_iGeneratorOffFaultCounter, m_iButtonLongPressCounter;
        int m_iAckFrameFlag;
        int m_iFrameInProcess; 
        // float fLineFaultCurVal1,fLineFaultCurVal2;
        
        /*Functions*/
        void checkForButtonEvent(void);
        void readConfiguration(void);
        time_t sendPing(void);
        void uploadframeFromBackUp(void);
        void getDeviceInfoFromServer(void);
        void checkVdiffFrame(void);
        //void checkUpdateCurrentRef(void);
        void updateJsonAndSendFrame(void);
        void staLEDHandler(void);
        void AppTimerHandler100ms(void);
		void checkForButtonLongPress(void);
        void readDeviceConfig(void);   

    public:
        uint8_t m_u8AppConter1Sec;

        /*construct*/
        cApplication();
        /*Destruct*/
        ~cApplication();

        /*Functions*/
        int appInit(void);
        void applicationTask(void);
        void frameHandlingTask(void);
        void commandParseTask(void);
        void RfidTask(void);
        void feedWdtInSafeMode(void);
        void GenDispVal(void);   
        void AppWatchdogInit(TaskHandle_t *taskhandle1, TaskHandle_t *taskhandle2);
        void AppWatchdogInit(TaskHandle_t *taskhandle1, TaskHandle_t *taskhandle2, TaskHandle_t *taskhandle3);
        void AppWatchdogInit(TaskHandle_t *taskhandle1, TaskHandle_t *taskhandle2, TaskHandle_t *taskhandle3, TaskHandle_t *taskhandle4);
};
#endif
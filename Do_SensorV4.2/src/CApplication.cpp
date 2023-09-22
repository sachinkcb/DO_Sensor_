#include "CApplication.h"
#include <mDash.h>
#include <WiFi.h>
#include "DISPLAY.h"
#include "BSP.h" 
#include "CRfid.h"
#include "MFRC522.h"


#define SERIAL_DEBUG
#ifdef SERIAL_DEBUG
#define debugPrint(...) Serial.print(__VA_ARGS__)
#define debugPrintln(...) Serial.println(__VA_ARGS__)
#define debugPrintf(...) Serial.printf(__VA_ARGS__)
#define debugPrintlnf(...) Serial.println(F(__VA_ARGS__))
#define WAITTIME 10
#else
#define debugPrint(...)    //blank line
#define debugPrintln(...)  //blank line
#define debugPrintf(...)   //blank line
#define debugPrintlnf(...) //blank line
#define WAITTIME 60 
#endif

#define HOOTER_ON 0 
#define WIFI_PASS "Infi@2016" /*Changed for Testing default value Infi@2016 */
#define MAX_SSID 10
/*Objects initialization*/
cBsp bsp;
Ticker AppTimer;
Ticker statusLED;
Ticker Buzzer;

cPCF85063A RTC;  //define a object of PCF85063A class
cCAT24C32 EEPROM;
FILESYSTEM fileSystem;
CBackupStorage backupStore;
cHTTP myHttp;
CDeviceConfig m_oConfig;
cMYDISPLAY Disp;  //initialize the Display class
cDOSensor m_doSensor;
cApplication m_display;
CPond m_oPdata;
CPondConfig m_oPondConfig;
RFIDReader rfidReader;

bool isOnline = false;
bool RTCSyncNow = false;
bool pingNow = false;
bool testRelayFlag = false;
bool RfidFlag = false;

int buzzCnt = 0;
int8_t GenDispCnt = 0;
uint8_t CommnFltFlag = 0;
uint8_t boardPower = 0;
uint8_t prevDate = 0;
int sendFrameType = NO_FRAME;
int isActiveTime = 0; 
int rebootAfterSetDataCmd = -1;
int rebootAfterOfflineCnt = 0;
uint8_t getSiteIdCntr = 0;
int m_iNetCheckCounter = 0;
uint8_t ssidIndex = 0;
uint8_t defssidIndex = 0;
const char SSID[MAX_SSID][20] = {"Nextaqua_EAP110", "Nextaqua_EAP110_1", "Nextaqua_EAP110_2", "Nextaqua_EAP110_3", "Nextaqua_EAP110_4",
                                "Nextaqua_EAP110_5", "Nextaqua_EAP110_6", "Nextaqua_EAP110_7", "Nextaqua_EAP110_8" ,"Nextaqua_EAP110_9"};
/*Changed for Testing*/
/******************
*   constructor
*******************/
cApplication::cApplication()
{
    m_u8SendBackUpFrameConter = 0;
    m_iTimeOutFrameConter = 0;
    m_iRtcSyncCounter = 0;
    m_iFrameInProcess = NO_FRAME;
    m_iButtonLongPressCounter = -1;
}

/*****************
*   Destruct
******************/
cApplication::~cApplication()
{

}
/****************************************************************
*   Safe copy copy with Overflow protection
*****************************************************************/
static void safeStrcpy(char * destination, const char * source, int sizeofDest)
{
  strncpy(destination,source,sizeofDest-1);
  destination[sizeofDest-1] = 0;
}

/****************************************************************
*   When we're reconnected, report our current state to shadow
*****************************************************************/
static void onConnected(void *event_data, void *user_data)
{
    debugPrintln("onConnected....");
    isOnline = true;
    pingNow = true;
}

/*****************************************************
*   When we're disconnected it comes to this event
******************************************************/
static void onDisConnected(void *event_data, void *user_data)
{
    debugPrintln("DisConnected.. :-(");
    isOnline = false;
}

/************************************************************************
*   Wifi setup function which is called by the mDash library
*   wifi_network_name-> string holds wifi name
*   wifi_pass-> string holds wifi password
*************************************************************************/
static void init_wifi(const char *wifi_network_name, const char *wifi_pass)
{
    if (wifi_network_name == NULL)
    {
        WiFi.softAP(MDASH_APP_NAME, "");
    }
    else
    {
        WiFi.begin(wifi_network_name, wifi_pass);
    }
}

/**************************************************************
*   RPC Function to set the Device config command
*   r-> pointer holds the config data buffer
***************************************************************/
static void RPChandler_setDevdata(struct jsonrpc_request *r)
{
    char buff[300];
    snprintf(buff, r->params_len + 1, "%S", (wchar_t *)r->params);
    debugPrintln("@@ Inside setDevdata...");
    debugPrintln(buff);

    /*Check Cmd Version, if Version is less or equal to previous Version Then return from here*/
    static double preVersn = 0;
    static double Versn = 0;
    mjson_get_number(r->params, r->params_len, "$.version", &Versn);
    if (Versn > preVersn)
    {
        debugPrintln(Versn);
        preVersn = Versn;
    }
    else
    {
        debugPrintln("Invalid version...");
        jsonrpc_return_success(r, "{\"statusCode\":300,\"statusMsg\":\"fail.\"}");
        return;
    }

    /*set SiteId*/
    char siteId[20];
    if (mjson_get_string(r->params, r->params_len, "$.siteId", siteId, sizeof(siteId)) != -1)
    {
        safeStrcpy(m_oConfig.m_cSiteId, siteId, sizeof(m_oConfig.m_cSiteId));
        mDashConfigSet("siteId", m_oConfig.m_cSiteId);
    }

    /*set Name*/
    char Name[10];
    if (mjson_get_string(r->params, r->params_len, "$.Name", Name, sizeof(Name)) != -1)
    {
        safeStrcpy(m_oConfig.m_cDeviceName, Name, sizeof(m_oConfig.m_cDeviceName));
        mDashConfigSet("deviceName", m_oConfig.m_cDeviceName);
    }

    double Index = -1;
    if (mjson_get_number(r->params, r->params_len, "$.DefaultSSID", &Index))
    {
        if(Index >= 0 && Index < MAX_SSID)
        {
            ssidIndex = (int)(Index);
            mDashConfigSet("DefaultSSID",String(ssidIndex).c_str());
        }
    }

    jsonrpc_return_success(r, "{\"statusCode\":200,\"statusMsg\":\"success.\"}");
    /*restart ESP after setting device data from portal/RPC*/
    rebootAfterSetDataCmd = 100;
}
/**************************************************************
*   RPC Function to clear Backup files
*   r-> pointer holds the config data buffer
***************************************************************/
static void RPChandler_ClearBackupFiles(struct jsonrpc_request *r)
{
  backupStore.clearAllFiles(&fileSystem);
  jsonrpc_return_success(r, "{\"statusCode\":200,\"statusMsg\":\"success.\"}");
}

/*****************************************************************
*   Function handler to receive rpc commands to set Safe Mode
*   r-> pointer holds the config data buffer
******************************************************************/
static void RPChandler_runSafeMode(struct jsonrpc_request *r)
{
    m_oConfig.m_bIsSafeModeOn = true;
    jsonrpc_return_success(r, "{\"statusCode\":200,\"statusMsg\":\"success.\"}");
}

/**************************************************************
*   RPC Function to know the device data 
*   r-> pointer holds the config data buffer
***************************************************************/
static void RPChandler_whoAreYou(struct jsonrpc_request *r)
{
    StaticJsonDocument<512> doc;
    debugPrintln("@@ Inside whoAreYou...");
    doc["deviceId"] = m_oConfig.m_cDeviceId;
    doc["Name"] = m_oConfig.m_cDeviceName;
    doc["siteId"] = m_oConfig.m_cSiteId;
    doc["deviceType"] = 16;
    doc["fwVer"] = FW_VERSION;
    doc["pcbMake"] = bsp.pcbMake;
    doc["currentEpoch"] = bsp.ConvertEpoch(&RTC);
    doc["ResetReason"] = m_oConfig.espResetReason;
    doc["Wifissid"] = WiFi.SSID();
    doc["rssi"] = WiFi.RSSI();
    doc["DefaultSSID"] = SSID[defssidIndex];

    char result[512];
    serializeJson(doc, result);
    debugPrintln(result);
    jsonrpc_return_success(r, "%s", result);
}
/**************************************************************
*   RPC Function to know the device data 
*   r-> pointer holds the config data buffer
***************************************************************/
static void RPChandler_getDoData(struct jsonrpc_request *r)
{
    StaticJsonDocument<512> doc;
    debugPrintln("@@ Inside getDodata...");
    doc["deviceId"] = m_oConfig.m_cDeviceId;
    doc["Name"] = m_oConfig.m_cDeviceName;
    doc["Temp"] =  m_doSensor.m_ftemperature;
    doc["DoData"] = m_doSensor.m_foxygen;
    doc["TempUnit"] = m_doSensor.m_stempUnit;
    doc["DOUnit"] = m_doSensor.m_soxygenUnit;

    char result[512];
    serializeJson(doc, result);
    debugPrintln(result);
    jsonrpc_return_success(r, "%s", result);
}
/**************************************************************
*   RPC Function to know the PondName 
*   r-> pointer holds the config data buffer
***************************************************************/
static void RPChandler_getPondName(struct jsonrpc_request *r)
{
    StaticJsonDocument<512> doc;
    debugPrintln("@@ Inside getPondName...");
    /*Get Pond Name*/
    // Parse the pond_id argument from the request
    double pondId = -1;
    mjson_get_number(r->params, r->params_len, "$.pondId", &pondId);
    char pond_name[10] = "";
    // Look up the corresponding pond name
    for (int i = 0; i <= TOTAL_PONDS; i++)
    {
        if (m_oPondConfig.m_oPondSettingList[i].m_u64PondId == (uint64_t)pondId)
        {
            sprintf(pond_name, "%s", m_oPondConfig.m_oPondSettingList[i].m_cName);
            break;
        }
    }
    doc["PondName"] = pond_name;
    doc["PondId"] = pondId;
    char result[512];
    serializeJson(doc, result);
    debugPrintln(result);
    jsonrpc_return_success(r, "%s", result);
}

/**************************************************************
*  Sync RTC
***************************************************************/
static void RPChandler_syncRTC(struct jsonrpc_request *r)
{
    RTCSyncNow = true;
    jsonrpc_return_success(r, "{\"statusCode\":200,\"statusMsg\":\"success.\"}");
}
/**************************************************************
* Generate V frame Instantlly
***************************************************************/
static void RPChandler_refreshFrame(struct jsonrpc_request *r)
{
    if (sendFrameType != CALL_FRAME)
    {
        sendFrameType = VDIFF_FRAME;
        jsonrpc_return_success(r, "{\"statusCode\":200,\"statusMsg\":\"success.\"}");
    }
    else
    {
        jsonrpc_return_success(r, "{\"statusCode\":300,\"statusMsg\":\"fail.\"}");
    }
}
/**************************************************************
* Set PCB Make
***************************************************************/
static void RPChandler_setpcbMake(struct jsonrpc_request *r)
{
    /*set clampType*/
    double pcbMake = -1;
    if (mjson_get_number(r->params, r->params_len, "$.pcbMake", &pcbMake))
    {
        if(pcbMake != -1)
        {    
            bsp.setPcbMake(pcbMake);
            mDashConfigSet("pcbMake", String(pcbMake).c_str());
            rebootAfterSetDataCmd = 100;
            jsonrpc_return_success(r, "{\"statusCode\":200,\"statusMsg\":\"success.\"}");
        }
    }
    else
    {
        jsonrpc_return_success(r, "{\"statusCode\":300,\"statusMsg\":\"fail.\"}");
    }
}

/**************************************************************
* Set defaultSSID
***************************************************************/
static void RPChandler_setdefaultSSID(struct jsonrpc_request *r)
{

    double index = -1;
    if (mjson_get_number(r->params, r->params_len, "$.DefaultSSID", &index))
    {
        if(index >= 0 && index < MAX_SSID)
		{
			ssidIndex = (int)index;
            defssidIndex = ssidIndex;
        	mDashConfigSet("DefaultSSID", String(ssidIndex).c_str());
        	rebootAfterSetDataCmd = 100;
            jsonrpc_return_success(r, "{\"statusCode\":200,\"statusMsg\":\"success.\"}");
		}
    }
    else
    {
        jsonrpc_return_success(r, "{\"statusCode\":300,\"statusMsg\":\"fail.\"}");
    }
}

/**************************************************************
*   RPC Function to set the Pond setting
*   r-> pointer holds the config data buffer
***************************************************************/
static void RPChandler_setPondConfig(struct jsonrpc_request *r)
{
    char rdata[2000] = "";
    snprintf(rdata, r->params_len + 1, "%s", (char *)r->params);
    debugPrintln("@@ Inside setPondConfig...");
    debugPrintln(rdata);
    static double PondsetVerInFrame = 0;    
    mjson_get_number(r->params, r->params_len, "$.PondSettingVer", &PondsetVerInFrame);
    debugPrintf("version1 set:%u\r\n",PondsetVerInFrame);
    debugPrintf("version2 set:%u\r\n",m_oPondConfig.m_dPondSettingVer);
    if (PondsetVerInFrame > m_oPondConfig.m_dPondSettingVer)
    {
        debugPrintln(PondsetVerInFrame);
        m_oPondConfig.m_dPondSettingVer = PondsetVerInFrame;
        fileSystem.writeFile(FNAME_PONDSETTING,rdata);
        jsonrpc_return_success(r, "{\"statusCode\":200,\"statusMsg\":\"success.\"}"); 
        m_oPondConfig.loadPondConfig(&fileSystem);       
    }
    else
    {
        jsonrpc_return_success(r, "{\"statusCode\":300,\"statusMsg\":\"fail.\"}");
    }
}
/*****************************************************
*   Function to intialize rpc Function Handlers
******************************************************/
void mDashFunctionInitialization(void)
{
    debugPrint("Inside MdashInit Conneting to :");
    debugPrintln(SSID[ssidIndex]);
    mDashBeginWithWifi(init_wifi, SSID[ssidIndex], WIFI_PASS, NULL);                 
    mDashRegisterEventHandler(MDASH_EVENT_CLOUD_CONNECTED, onConnected, NULL); 
    mDashRegisterEventHandler(MDASH_EVENT_NETWORK_LOST, onDisConnected, NULL);   
    jsonrpc_export("ActivateSafeMode", RPChandler_runSafeMode, NULL);
}

void mDashAllRpcFunctionInit(void)
{
    
    jsonrpc_export("getPondName", RPChandler_getPondName, NULL);
    jsonrpc_export("setPondConfig", RPChandler_setPondConfig, NULL);
    jsonrpc_export("setDevdata", RPChandler_setDevdata, NULL);
    jsonrpc_export("ClearBackupFiles", RPChandler_ClearBackupFiles, NULL);
    jsonrpc_export("getLiveFrame",RPChandler_refreshFrame,NULL);
    jsonrpc_export("setpcbMake", RPChandler_setpcbMake, NULL);
    jsonrpc_export("setDefaultSSID",RPChandler_setdefaultSSID,NULL); 
    jsonrpc_export("syncRTC", RPChandler_syncRTC,NULL);
    jsonrpc_export("whoAreYou", RPChandler_whoAreYou, NULL); 
    jsonrpc_export("GetDoData", RPChandler_getDoData, NULL);
}

/*****************************************************************************
*   Function to intialize internal watchdog
*   taskhandler -> Indicates for which hamdler wdt should be initialized
*****************************************************************************/
void cApplication::AppWatchdogInit(TaskHandle_t *taskhandle1, TaskHandle_t *taskhandle2)
{
    bsp.wdtInit();
    bsp.wdtAdd(*taskhandle1);
    bsp.wdtAdd(*taskhandle2);
}

void cApplication::AppWatchdogInit(TaskHandle_t *taskhandle1, TaskHandle_t *taskhandle2, TaskHandle_t *taskhandle3)
{
    bsp.wdtInit();
    bsp.wdtAdd(*taskhandle1);
    bsp.wdtAdd(*taskhandle2);
    bsp.wdtAdd(*taskhandle3);
}

void cApplication::AppWatchdogInit(TaskHandle_t *taskhandle1, TaskHandle_t *taskhandle2, TaskHandle_t *taskhandle3, TaskHandle_t *taskhandle4)
{
    bsp.wdtInit();
    bsp.wdtAdd(*taskhandle1);
    bsp.wdtAdd(*taskhandle2);
    bsp.wdtAdd(*taskhandle3);
    bsp.wdtAdd(*taskhandle4);
}

/*******************************************
*   Timer handler for led blink
*******************************************/
void cApplication::staLEDHandler(void)
{
    static int ckCnt = 0;
    static int rssi = 0;
    static int ledCnt = 0;

    ckCnt++;
    ledCnt++;
    if (ckCnt >= 50) //5 sec
    {
        ckCnt = 0;
        rssi = WiFi.RSSI();
    }
    if (isOnline && myHttp.m_bIsConnected)
    {
        if (ledCnt >= ((rssi / 10) * (-1)))
        {
            ledCnt = 0;
            bsp.indLedToggle();
        }
    }
    else
    {
        bsp.indLedOff();
    }
}

/*******************************************
*   Timer handler for 100ms interrupt
*******************************************/
void cApplication::AppTimerHandler100ms(void)
{
    m_u8AppConter1Sec++;
}

/************************************************* 
*   read Config From MDashCfg File
*************************************************/
void cApplication::readDeviceConfig(void)
{
    char val[20];
    int ret = 0;
    
    /* read Site Id*/
    ret = mDashConfigGet("siteId", val, sizeof(val));    
    if (ret == 0)
    {
        safeStrcpy(m_oConfig.m_cSiteId, val,sizeof(m_oConfig.m_cSiteId));
        debugPrint("Found, siteId = ");
        debugPrintln(m_oConfig.m_cSiteId);
    }
    else if (ret < 0)
    {
        strcpy(m_oConfig.m_cSiteId, "DEFAULTSITE");
        debugPrint("Not Found, siteId = ");
        debugPrintln(m_oConfig.m_cSiteId);
    }

    /* read deviceId*/
    ret = mDashConfigGet("deviceId", val, sizeof(val));
    if (ret == 0)
    {
        safeStrcpy(m_oConfig.m_cDeviceId, val,sizeof(m_oConfig.m_cDeviceId));
        debugPrint("Found, deviceId = ");
        debugPrintln(m_oConfig.m_cDeviceId);
    }
    else if (ret < 0)
    {
        debugPrint("Not Found, deviceId = ");
        strcpy(m_oConfig.m_cDeviceId, "device0");
        debugPrintln(m_oConfig.m_cDeviceId);
    }
    
    /*Read Device Name From mConfg File*/
    ret = mDashConfigGet("deviceName", val, sizeof(val));
    if (ret == 0)
    {
        safeStrcpy(m_oConfig.m_cDeviceName, val,sizeof(m_oConfig.m_cDeviceName));
        debugPrint("Found, deviceName = ");
        debugPrintln(m_oConfig.m_cDeviceName);
    }
    else if (ret < 0)
    {
        strcpy(m_oConfig.m_cDeviceName, "W0");
        debugPrint("Not Found, deviceName = ");
        debugPrintln(m_oConfig.m_cDeviceName);
    }

    ret = mDashConfigGet("DefaultSSID", val, sizeof(val));
    if (ret == 0)
    {
        ssidIndex = String(val).toInt();
        debugPrint(ssidIndex);
        /*Validate*/
        if(ssidIndex < 0 || ssidIndex > (MAX_SSID-1)) ssidIndex = 0;
        defssidIndex = ssidIndex;
        debugPrint("Found, SSID Index = ");
        debugPrintln(ssidIndex);
    }
    else if (ret < 0)
    {
        ssidIndex = 0;
        defssidIndex = ssidIndex;
        debugPrint("Not Found, SSID Index = ");
        debugPrintln(ssidIndex);
    }

}
/*********************************************************
*   Function to read config file data
**********************************************************/
void cApplication::readConfiguration(void)
{
    /*Reading device configuration data from mdash Config*/
    readDeviceConfig();
}

/**************************************************************
* Function to check and complete cApplication related tasks
***************************************************************/
int cApplication::appInit(void)
{   
    /*With Display*/
    uint8_t Wait = 0;
    char fwver[10] = {0};
    char id[5] = {0};
    char ssidStr[10] = {0};
    /*Serial debug bin*/
    Serial.begin(115200);
    Serial1.begin(9600,SERIAL_8N1,13,32);
    Serial1.println("Hello DO Sensor");
    /*Reading esp reset reason*/
    m_oConfig.espResetReason = esp_reset_reason();
    /*File System initialization*/
    fileSystem.begin();
    /*Backup storage initialization*/
    backupStore.InitilizeBS(&fileSystem);
    /*Read Configurations from mDash.cfg File*/
    readDeviceConfig();
    /*mdash call and handlers initialization*/
    mDashFunctionInitialization();
    /*Bsp GPIO Initalization*/
    bsp.gpioInitialization();
    /*to know device is rebooted*/
    m_oConfig.m_u8IsReboot = 1;
    bsp.displayInit(&Disp);
    bsp.hooterInit();
    /*Delay to make all the intialzation complete and display version number*/
    safeStrcpy(id,m_oConfig.m_cDeviceId+6,sizeof(id));
    sprintf(fwver,"V-%d", FW_VERSION);
    sprintf(ssidStr, "SS1d%d", defssidIndex);
    int st = 0;
    while (Wait <= WAITTIME) 
    {        
        delay(1000);
        bsp.wdtPinToggle();
        Wait++;
        if (m_oConfig.m_bIsSafeModeOn)
        {
            return 0;
        }
        if(st == 0)
        {
            Disp.printString((uint8_t *)fwver,0,5);
        }
        else if(st == 1)
        {
            Disp.printString((uint8_t *)id,0,5);
        }
        else if(st == 2)
        {
            Disp.printString((uint8_t *)ssidStr, 0, 5);
        }
        st++;
        if(st > 2) st = 0;
        debugPrint("Wait : ");
    }
    mDashAllRpcFunctionInit();
    /* Spi initialization*/
    bsp.spiInitialization();
    /*I2C initialization*/
    bsp.i2cInitialization(); 
    /*Initialize RFID Reader*/
     rfidReader.init(BSP_CS,RF_READER_RST); 
     debugPrintln("rfid initialized");
    /*Watchdog pin toggling*/
    bsp.wdtPinToggle();
    /*Iniatlization for application timer*/
    AppTimer.attach(0.1, +[](cApplication* App) { App->AppTimerHandler100ms(); }, this);
    /*Initialization of status led timer handler*/
    statusLED.attach(0.1, +[](cApplication* Led) { Led->staLEDHandler(); }, this);
    /*Load Pond data from File*/
    m_oPondConfig.loadPondConfig(&fileSystem); 
    return 1;
}

/********************************************************************************************
*   Function for checking if button pressed and making operation according to button pressed
**********************************************************************************************/
void cApplication::checkForButtonEvent(void)
{
    uint8_t buttonCode = bsp.getButtonEvent();
    if(buttonCode != 0xFF)
    {
        /*For any button presssed switch off hooter*/
        debugPrintln(buttonCode);
    }

    switch (buttonCode)
    {
        case BUTTONUP:
            debugPrintln("BUTTONUP");
            // m_iButtonLongPressCounter = 0;
            break;
        case BUTTONDOWN:
            debugPrintln("BUTTONDOWN");
            // m_iButtonLongPressCounter = 0;
            break;
        case BUTTONSEL:
            debugPrintln("BSP_But_Select");
            m_iButtonLongPressCounter = 0;
            break;
        case BUTTONM1ON:
            sendFrameType = VDIFF_FRAME;
            buzzCnt = 10;
            break;
        case BUTTONM1OFF:            
            break;
        default:
            break;
    }
    /*Check for long press event*/
    checkForButtonLongPress();
}

/****************************************************************
*   Function to check if button pressed for long time  more than 3 sec
*****************************************************************/
void cApplication::checkForButtonLongPress(void)
{
    if(m_iButtonLongPressCounter >= 0) 
        m_iButtonLongPressCounter++;

    if(m_iButtonLongPressCounter >= 30)
    {
        // if((!bsp.ioPinRead(BSP_BTN_1)) && (!bsp.ioPinRead(BSP_BTN_2)))
        // {
        //     debugPrintln("BSP_BTN_1 and BSP_BTN_2 is pressed of 3 seconds");
        //     delay(2000);
        //     ESP.restart();
        // } 
        // else if(!bsp.ioPinRead(bsp.enterButtonPin))
        // {
        //     debugPrintln("Enterkay is pressed of 3 seconds");
        //     testRelayFlag = true;
        // }
        m_iButtonLongPressCounter = -1;
    }
}
/**************************************************
*   Function to complete cApplication related tasks 
***************************************************/
void cApplication::applicationTask(void)
{    
    bsp.wdtfeed();
    /*Reset Device on receipt of devicedata ,like Clamps type Siteid,name*/
    if(rebootAfterSetDataCmd > 0)
    {
        rebootAfterSetDataCmd--;
        debugPrint("espRestart In : ");  debugPrint(rebootAfterSetDataCmd); debugPrintln(" secs");
        if(rebootAfterSetDataCmd <= 0)
        {
            ESP.restart();
        }
    }

    /*Function to check if button is pressed or not*/
    checkForButtonEvent();    
    /*Feed external watchdog */
    bsp.wdtPinToggle();    
    /* Allow to set credentials over the serial line */
    if (Serial.available() > 0) mDashCLI(Serial.read());

    /*one Sec task*/
    if (m_u8AppConter1Sec >= 10)
    {
        m_u8AppConter1Sec = 0;

        /*Read Time from RTC*/
        time_t now =  bsp.ConvertEpoch(&RTC);
        //debugPrintln(now);
        if(now) m_oConfig.m_tEpoch = now; 
        
         m_display.GenDispVal();

        m_iTimeOutFrameConter++;
        m_iRtcSyncCounter++;      

        if ((isOnline == false) || (myHttp.m_bIsConnected == false))
        {
            rebootAfterOfflineCnt++;
            m_iNetCheckCounter++;
        }
        else
        {
            rebootAfterOfflineCnt = 0;
            m_iNetCheckCounter = 0; 
        }
        /*Function to check for value difference or faults*/
       // checkVdiffFrame();   
    }
}
/*******************************************************************************
 * Send ping frame to server to check if device is connected to Nextaqua server
 *******************************************************************************/
time_t cApplication::sendPing(void)
{    
    if (isOnline)
    {
        char frame[256];
        sprintf(frame,"{\"ReasonForPacket\":\"Ping\",\"siteId\":\"%s\",\"epochTime\":%ld,\"deviceId\":\"%s\"}",m_oConfig.m_cSiteId,m_oConfig.m_tEpoch,m_oConfig.m_cDeviceId);
        debugPrintln(frame);
        return myHttp.uploadPingFrame(frame);
    }
    return 0;
}
/***********************************************************
*   Send data from backup storage periodically if available
************************************************************/
void cApplication::uploadframeFromBackUp(void)
{
    if (isOnline)
    {
        if (myHttp.m_bHttpBusy)
        {
            debugPrintln("httpBusy :-(");
            return;
        }
        if (!myHttp.m_bIsConnected || pingNow)
        {
            pingNow = false;
            debugPrintln("Trying ping in BAK");
            sendPing();
            return;
        }
        if (backupStore.available())
        {            
            char fdata[1024] = {0};
            debugPrintln("Available..");
            backupStore.readFromBS(&fileSystem, fdata);
            if (myHttp.uploadDataFrame(fdata)) //
            {
                backupStore.moveToNextFile(&fileSystem);
            }
        }
    }
}

/***********************************************************************************************
*  
************************************************************************************************/
void cApplication::checkVdiffFrame(void)
{
    //sendFrameType = VDIFF_FRAME;
}

/*****************************************************************
*   Function to update the Json string and upload the frame
******************************************************************/
void cApplication::updateJsonAndSendFrame(void)
{
    debugPrint("## Generate And send frame");
    /*Check if RTC time is correct*/
    if ((m_oConfig.m_tEpoch < 1609439400))
    {
        //1609439400 is Friday, January 1, 2021 12:00:00 AM GMT+05:30
        debugPrint("## Epoch Miss Match,Wrong Year : Sync RTC");
        RTCSyncNow = true;
        m_iTimeOutFrameConter = 0;
        m_iFrameInProcess = NO_FRAME;
        return;
    }

    DynamicJsonDocument Data(2000);
   /*****************************************************/
    Data["Name"] = m_oConfig.m_cDeviceName;
    Data["deviceId"] = m_oConfig.m_cDeviceId;
    Data["siteId"] = m_oConfig.m_cSiteId;
    Data["fwVer"] = FW_VERSION;
    Data["devicetype"] = "DO";
    Data["isReboot"] = -1;
    if (m_oConfig.m_u8IsReboot)
    {   
        m_oConfig.m_u8IsReboot = 0;
        Data["isReboot"] = m_oConfig.espResetReason;
    }
    Data["Wifissid"] = WiFi.SSID();
    Data["rssi"] = WiFi.RSSI();
    Data["ReasonForPacket"] = "V";
    Data["epochTime"] = m_oConfig.m_tEpoch;   
    Data["userName"] = m_oConfig.m_cUserName;
    Data["isRtcSynced"] = 0;
    Data["PondName"] = m_doSensor.pondname;
    Data["tagId"] = m_doSensor.tagId;
    Data["pid"] = "123456";

    if(sendFrameType == TOUT_FRAME)
    {
        Data["ReasonForPacket"] = "T";
        m_iFrameInProcess = sendFrameType;
    }
    Data["isHistory"] = LIVE_FRAME;
    /**********    Device Specific data    *******/
    Data["Temp"] =  m_doSensor.m_ftemperature;
    Data["DOData"] = m_doSensor.m_foxygen;
    Data["TempUnit"] = m_doSensor.m_stempUnit;
    Data["DOUnit"] = m_doSensor.m_soxygenUnit;
    /***************End of Device Data  ***********/
    /*Try to send frame if device is online of save to backup memory*/
    char frame[2000];
    if (isOnline)
    {
        serializeJson(Data, frame);
        debugPrint(frame); 
        if (!myHttp.uploadDataFrame(frame) && sendFrameType == VDIFF_FRAME)
        {
            Data["isHistory"] = HISTORY_FRAME;
            serializeJson(Data, frame);
            backupStore.writeInBS(&fileSystem, frame);
        }       
    }
    else if(sendFrameType == VDIFF_FRAME)
    {
        Data["isHistory"] = HISTORY_FRAME;
        serializeJson(Data, frame);
        backupStore.writeInBS(&fileSystem, frame);
    }
    sendFrameType = NO_FRAME;
    /*Reset Timeout frame counter,Push Timeout Frame*/
    m_iTimeOutFrameConter = 0;
    m_iFrameInProcess = NO_FRAME;
}

/*********************************************************************
 * When site is not configured , get the deveoce data from server
 * read and write the device data in mdash.cnfg file update siteId and deviceName
**********************************************************************/
void cApplication::getDeviceInfoFromServer(void)
{
    if ((strcmp(m_oConfig.m_cSiteId, "DEFAULTSITE") == 0) && (isOnline))
    {
        char frame[256];
        sprintf(frame, "{\"deviceId\":\"%s\"}", m_oConfig.m_cDeviceId);
        debugPrintln(frame);
        if (myHttp.getDeviceInfo(frame))
        {
            char strdata[20];
            /*Get Site Id*/
            int ret = mjson_get_string(&myHttp.m_sPayload[0], myHttp.m_sPayload.length(), "$.siteId", strdata, sizeof(strdata));
            if (ret == -1)
            {
                strcpy(strdata, "DEFAULTSITE");
            }
            safeStrcpy(m_oConfig.m_cSiteId, strdata, sizeof(m_oConfig.m_cSiteId));
            mDashConfigSet("siteId", m_oConfig.m_cSiteId);
            debugPrint("site Id : ");
            debugPrintln(m_oConfig.m_cSiteId);

            /*Get Device Name*/
            ret = mjson_get_string(&myHttp.m_sPayload[0], myHttp.m_sPayload.length(), "$.name", strdata, sizeof(strdata));
            if (ret == -1)
            {
                strcpy(strdata, "W0");
            }
            safeStrcpy(m_oConfig.m_cDeviceName, strdata, sizeof(m_oConfig.m_cDeviceName));
            mDashConfigSet("deviceName", m_oConfig.m_cDeviceName);
            debugPrint("device Name : ");
            debugPrintln(m_oConfig.m_cDeviceName);

            double index = -1;
            if (mjson_get_number(&myHttp.m_sPayload[0], myHttp.m_sPayload.length(), "$.DefaultSSID", &index))
            {
                if(index >= 0 && index < MAX_SSID)
                {
                    ssidIndex = (int)(index);
                    mDashConfigSet("DefaultSSID", String(ssidIndex).c_str());
                }
            }

            /*restart ESP after setting device data from portal/RPC*/
            rebootAfterSetDataCmd = 100;
        }
        else
        {
            debugPrintln("failed to send..:-(");
        }
    }
}

/*******************************************************
*   Function for Switch wifi Networks
*********************************************************/
void switchWifi(void)
{
    debugPrintln("@@ Switch Wifi network....");
    debugPrint("@@ New Wifi SSID: ");
    debugPrintln(SSID[ssidIndex]);   
    WiFi.disconnect();
    WiFi.begin(SSID[ssidIndex], WIFI_PASS);
    /*Increament after retrying to give moretime to default ssid*/
    /*Try other ssids when default SSID is zero*/
    if(defssidIndex == 0)
    {
        ssidIndex++;
        if(ssidIndex > (MAX_SSID-1)) ssidIndex = 0;
    }
}

/*******************************************************
*   Function for frame handling related tasks
*********************************************************/
void cApplication::frameHandlingTask(void)
{
    bsp.wdtfeed();    
    /*When device offline for more than 15 min reset modem*/
    if(rebootAfterOfflineCnt > 900) // Must be 900 sec 15 min
    {
        rebootAfterOfflineCnt = 0;
        debugPrintln("@@ Reset Modem Offline more than 15 min...");
        sendFrameType = VDIFF_FRAME;
        bsp.wdtfeed();
        delay(2000);
        /*reset SSID to deafult*/
        ssidIndex = defssidIndex;
        WiFi.disconnect();
        WiFi.begin(SSID[ssidIndex], WIFI_PASS);
        /*Reboot Esp32*/
        ESP.restart();
    }

    /*Update and send frame*/
    if((m_iFrameInProcess == NO_FRAME) && ((sendFrameType != NO_FRAME)))
    {
        m_iFrameInProcess = sendFrameType;
        updateJsonAndSendFrame();
    }
    bsp.wdtfeed();

    /*Check for multiple Wifi Networks when device is offline every 90sec*/
    if(m_iNetCheckCounter > 90)
    {
       m_iNetCheckCounter = 0;
       /*Removed for Testing*/
       //switchWifi(); 
    }
    
    //for Test
    /*timeout contr to send t frame*/
    if (m_iTimeOutFrameConter >= 300) //should be 300
    {
        debugPrintln("T T T Send Timeout Frame");
        if(sendFrameType == NO_FRAME)
        {
            sendFrameType = TOUT_FRAME;
        }
    } 
    /*Sends frame from backup if any, and if not connceted ping*/
    m_u8SendBackUpFrameConter++;
    if (m_u8SendBackUpFrameConter >= 10)
    {
        m_u8SendBackUpFrameConter = 0;
        //debugPrintln("S S S Send Backup Frame");
        uploadframeFromBackUp();        
    }

    bsp.wdtfeed();
    /*Sets the siteId and device name*/
    getSiteIdCntr++;
    if(getSiteIdCntr >= 60)
    {
        getSiteIdCntr = 0;
        getDeviceInfoFromServer();       
    }

    bsp.wdtfeed();

    /*Check and sync RTC every 10 MIN */
    if(m_iRtcSyncCounter >= 600 || RTCSyncNow)
    {
        if(isOnline)
        {
            debugPrintln("@@ Check and Sync RTC");
            /*Get server epoch in Local time*/
            time_t serverEpoch = sendPing();
            /*Convert local epoch to  GMT*/
            if (bsp.syncRTCTime(&RTC,serverEpoch, m_oConfig.m_tEpoch + 19800))
            {   
                m_iRtcSyncCounter = 0;
            }
            else
            {
                /*If Rtc sync is failed it will try to sync in one Min again*/
                m_iRtcSyncCounter = 540;
            }
            RTCSyncNow = false;            
        }
        else
        {
            m_iRtcSyncCounter = 540;
        }        
    }
}

/*****************************************************/
 /* General Display Function
 **************************************************** */
void cApplication::GenDispVal(void)
{
    switch (GenDispCnt)
    {
    case 1:
         Disp.printString((uint8_t *)"do   ", 0, 5);
        break;
    case 2:
          Disp.printFloat(m_doSensor.m_foxygen, 0,3,2);
        break;
    case 3:
        Disp.printString((uint8_t *)"tENNP", 0, 5);
        break;
    case 4:
        Disp.printFloat(m_doSensor.m_ftemperature, 0, 5,2);
       break;        
    }
    if ((GenDispCnt >= 0 && GenDispCnt <= 4))
    GenDispCnt++;
    else if(GenDispCnt >4)
    GenDispCnt = 0;
}

/**************************************************
*   Functionto feed wdt when running on safe mode
***************************************************/
void cApplication::feedWdtInSafeMode(void)
{
    if(m_oConfig.m_bIsSafeModeOn)
    {
        bsp.wdtPinToggle();
    }
}

void cApplication::commandParseTask(void)
{
    while(Serial1.available() > 0) m_doSensor.decode(Serial1.read());
    m_doSensor.ParseCommands();
    // if(m_doSensor.ParseCommands() == 0)
    //     debugPrint("Wrong data in frame...");
}

void cApplication::RfidTask(void)
{
	if (rfidReader.isTagPresent())
	{
		//new tag is available
        buzzCnt = 10;
        RfidFlag = true;
		debugPrintln("card detected");
		m_doSensor.tagId = rfidReader.readUID();
		debugPrintf("Tag no: %u\n",m_doSensor.tagId);
        debugPrintln();
		for (int i = 0; i <= TOTAL_PONDS; i++)
		{
			if (m_oPondConfig.m_oPondSettingList[i].m_u64PondId == (uint64_t)m_doSensor.tagId)
			{
				sprintf(m_doSensor.pondname,"%s",m_oPondConfig.m_oPondSettingList[i].m_cName);
				debugPrintf("pond name : %s\r\n",m_doSensor.pondname);
                debugPrintf("pond id : %u\r\n",m_doSensor.tagId);
                Disp.printString((uint8_t *)m_doSensor.pondname,0,5);
            }
		}
        debugPrintln("tag is read succesfully");
    }
    /*Operate Buzzer*/
    if(buzzCnt > 0)
    {
        /*Switch on Buzz*/
        digitalWrite(BUZZER_PIN, ON);
        buzzCnt--;
    }   
    else
    {
        digitalWrite(BUZZER_PIN, OFF);
    } 
}

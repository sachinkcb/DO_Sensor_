#include "CHttp.h"
#include "mDash.h"

// #define SERIAL_DEBUG
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

/*construct*/
cHTTP::cHTTP(){

}

/*destruct*/
cHTTP::~cHTTP(){

}

/************************************************************************* 
*   Function to upload frame to server using HTTP
*   Data-> Holds the buffer to upload
**************************************************************************/
uint8_t cHTTP::uploadDataFrame(char *Data)
{
    uint8_t ret = 0;
    if (!m_bHttpBusy)
    {
        m_bHttpBusy = true;
        debugPrint("[HTTP] IsConnected : "); debugPrintln(http.connected());
        debugPrintln("[HTTP] begin : Frame");
        debugPrintln(Data);
        /*New Url HTTP link for uploading data in firebase*/
        http.begin("http://us-central1-nextaqua-22991.cloudfunctions.net/processSensorsData");                    
        /*Specify content-type header*/
        http.addHeader("Content-Type", "application/json");   
        /* Added to resolve isssue with time out */
        http.setTimeout(30000);                                   

        int httpCode = http.POST(Data);
        debugPrint("httpCode: ");
        debugPrintln(httpCode);
        debugPrintln(http.errorToString(httpCode));
        
        if (httpCode == HTTP_CODE_OK)
        {
            debugPrintln("@@ Uploded to Nextaqua server :-)");
            ret = 1;
            m_bIsConnected = true;
        }
        else
        {
            debugPrintln("@@ failed to send..:-(");
            ret = 0;
            m_bIsConnected = false;
        }
        // String payload = http.getString();
        // debugPrintln(payload);
        http.end();
    }
    else
    {
        ret = 0;
        m_bIsConnected = false;
        debugPrintln("@@ HTTP Busy");
    }
    debugPrintln("[HTTP] end : Frame");
    m_bHttpBusy = false;
    return ret;
}

/************************************************************************* 
*   Function to upload the data into ping server
*   Data-> Holds the buffer to upload
**************************************************************************/
time_t cHTTP::uploadPingFrame(char *Data)
{
    time_t retEpoch = 0;
    if (!m_bHttpBusy)
    {
        m_bHttpBusy = true;
        debugPrint("[HTTP] IsConnected : Ping"); debugPrintln(http.connected());
        debugPrintln("[HTTP] begin : Ping");
        /*HTTP link for ping server and get Epoch*/
        http.begin("http://us-central1-nextaqua-22991.cloudfunctions.net/pingService"); 
        /*Specify content-type header*/
        http.addHeader("Content-Type", "application/json");   
        /* Added to resolve isssue with time out */
        http.setTimeout(30000);
        //debugPrintln(Data);
        int httpCode = http.POST(Data);
        debugPrintln(http.errorToString(httpCode));
        if (httpCode == HTTP_CODE_OK)
        {
            debugPrintln("@@ Uploded Ping Frame :-)");
            m_bIsConnected = true;
            /* Payload format {"statusCode":200,"statusMessage":"Success","serverEpoch":1625899085}*/
            String payload = http.getString();
            debugPrintln(payload);
            double num = -1;
            mjson_get_number(payload.c_str(),strlen(payload.c_str()),"$.serverEpoch",&num);
            if(num != -1)
            {
                /*To get Local epoch*/
                retEpoch = (time_t)(num + 19800); 
            }            
        }
        else
        {
            debugPrintln("@@ failed to send Ping -(");
            m_bIsConnected = false;
        }
        http.end();
    }
    else
    {
        m_bIsConnected = false;
        debugPrintln("@@ HTTP Busy");
    }
    debugPrintln("[HTTP] end : Ping");
    m_bHttpBusy = false;
    return retEpoch;
}

/************************************************************************* 
*   Function to read device data
*   Data-> Holds the buffer to upload
**************************************************************************/
uint8_t cHTTP::getDeviceInfo(char *Data)
{
    uint8_t ret = 0;
    if ((!m_bHttpBusy) && (m_bIsConnected))
    {
        m_bHttpBusy = true;
        debugPrintln("[HTTP] begin : GetD");
        /*Link*/
        http.begin("http://us-central1-nextaqua-22991.cloudfunctions.net/getSiteIdForDevice");
        http.setTimeout(30000);                                                                     
        http.addHeader("Content-Type", "application/json");                                          
        // debugPrintln(Data);
        int httpCode = http.POST(Data);
        // debugPrintln(http.errorToString(httpCode));        
        if (httpCode == HTTP_CODE_OK)
        {
            debugPrintln("@@ Requested getDevice data :-)");
            m_sPayload = http.getString();
            debugPrintln(m_sPayload);
            ret = 1;
            m_bIsConnected = true;
        }
        else
        {
            debugPrintln("@@ failed to send getDevice  :-(");
            ret = 0;
            m_bIsConnected = false;
        }
    }
    else
    {
        ret = 0;
        m_bIsConnected = false;
        debugPrintln("@@ HTTP Busy or Not connected");
    }
    http.end();
    debugPrintln("[HTTP] end : GetD");
    m_bHttpBusy = false;
    return ret;
}

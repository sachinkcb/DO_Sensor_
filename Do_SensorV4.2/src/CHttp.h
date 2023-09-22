#ifndef HTTP_H
#define HTTP_H

#include <stdint.h>
#include <HTTPClient.h>

class cHTTP{
    private:
        HTTPClient http; //HTTP Class
    public:
        bool m_bHttpBusy;
        bool m_bIsConnected;
        String m_sPayload;
        /*construct*/
        cHTTP(void);
        /*destruct*/
        ~cHTTP(void);
        /*functions*/
        uint8_t uploadDataFrame(char *);
        time_t uploadPingFrame(char *);
        uint8_t getDeviceInfo(char *);
};

#endif
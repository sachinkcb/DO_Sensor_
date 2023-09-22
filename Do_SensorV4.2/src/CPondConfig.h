#ifndef CPondConfig_H
#define CPondConfig_H

#include <stdint.h>
#include <FILESYSTEM.h>
#include "CPond.h"
#define TOTAL_PONDS 10
#define FNAME_PONDSETTING    "/PondSetting.txt"


class CPondConfig
{
    private:

    public:
        /* Construct */
        CPondConfig();
        /* Destruct */
        ~CPondConfig();
        CPond m_oPdata;
        uint8_t m_u8PondNum;
        CPond m_oPondSettingList[TOTAL_PONDS]; 
        double m_dPondSettingVer;
        int loadPondConfig(FILESYSTEM* filesystem);
        int readSetting(FILESYSTEM* filesystem,char *SettingData);
        int writeSetting(FILESYSTEM* filesystem);
};

#endif
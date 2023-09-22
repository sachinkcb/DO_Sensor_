#ifndef CPond_H
#define CPond_H

#include <stdint.h>

class CPond
{
    private:

    public:   
    /* Construct */
    CPond();
    /* Destruct */
    ~CPond(); 
    char m_cName[10];
    uint64_t m_u64PondId;
    char m_cPid[20];
};

#endif
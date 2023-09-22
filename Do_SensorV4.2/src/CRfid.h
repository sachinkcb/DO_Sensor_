#ifndef RFID_H
#define RFID_H

#include "BSP.h"
// #include "Adafruit_PN532.h"

// #define PN532_SCK  (2)
// #define PN532_MOSI (3)
// #define PN532_SS   (4)
// #define PN532_MISO (5)


// class NFCReader {
//   private:
//     Adafruit_PN532 nfc;
//   public:
//     NFCReader();
//     bool init();
//     bool readCard(byte* uid, uint8_t* uidLength);
// };

// #endif // RFID_H
// #ifndef RFIDReader_h
// #define RFIDReader_h

#include <SPI.h>
#include <MFRC522.h>


class RFIDReader {
  private:
    MFRC522 rfid;
    byte SS_PIN;
    byte RST_PIN;
    
  public:
    //RFIDReader(byte SS, byte RST);
    RFIDReader(){};
    void init(byte SS, byte RST);
    bool isTagPresent();
    uint64_t readUID();
};

#endif // RFIDReader_h

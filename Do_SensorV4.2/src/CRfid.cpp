#include "CRfid.h"

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

// NFCReader::NFCReader() : nfc(Adafruit_PN532( PN532_SCK, PN532_MISO, PN532_MOSI, PN532_SS )) {}

// //Implement the init method of the NFCReader class to initialize the PN532 object and check if the PN532 is connected:
// bool NFCReader::init() {
//   nfc.begin();
//   uint32_t versiondata = nfc.getFirmwareVersion();
//   if (!versiondata) {
//     debugPrint("PN532 not found");
//     return false;
//   }
//   // configure board to read RFID tags
//   nfc.SAMConfig();
//   return true;
// }

// //Implement the readCard method of the NFCReader class to read the unique ID of an NFC tag:

// bool NFCReader::readCard(byte* uid, uint8_t* uidLength) {
//   //Wait for an NFC tag
//   // // read the unique ID of the NFC tag
//   if (!nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, uidLength)) {
//     return false;
//   }
//   // tag was read successfully
//   return true;
// }

// RFIDReader::RFIDReader(byte SS, byte RST) :
//   rfid(SS, RST),
//   SS_PIN(SS),
//   RST_PIN(RST)
// {}

void RFIDReader::init(byte SS, byte RST) {
  rfid.PCD_Init(SS,RST); 
}

bool RFIDReader::isTagPresent() {
  return rfid.PICC_IsNewCardPresent();
}

uint64_t RFIDReader::readUID() {
    uint64_t decimal = 0;
  if (rfid.PICC_ReadCardSerial()) { // NUID has been readed
    MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
    debugPrint("RFID/NFC Tag Type: ");
    debugPrintln(rfid.PICC_GetTypeName(piccType));

    // print UID in Serial Monitor in the hex format
    debugPrint("UID:");
    for (int i = 0; i < rfid.uid.size; i++) {
      debugPrintf(rfid.uid.uidByte[i] < 0x10 ? " 0" : " ");
      debugPrint(rfid.uid.uidByte[i], HEX);
      decimal = decimal * 256 + rfid.uid.uidByte[i];
    }
    debugPrintln();
    debugPrint("Decimal value: ");
    debugPrintln(decimal);

   // Halt the tag to prevent reading it again too soon
    rfid.PICC_HaltA(); // halt PICC
    rfid.PCD_StopCrypto1(); // stop encryption on PCD
    return decimal;
  }
}

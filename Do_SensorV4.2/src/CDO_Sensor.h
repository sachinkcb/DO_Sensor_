#ifndef DISSOLVED_OXYGEN_METER_H
#define DISSOLVED_OXYGEN_METER_H

#include "CDeviceConfig.h"
#include "BSP.h"
#include "CPondConfig.h"

#define MAX_FRAME_SIZE 30


class DO_frame {

public:
 char mode;
 uint8_t polarity_;   // D14: polarity of the reading
 uint8_t lower_dp_;   // D10: decimal point for lower display
 uint8_t lower_val_;  // D8-D5: lower display reading
 uint8_t lower_unit_; // D13: annunciator for lower display unit
 uint8_t upper_dp_;   // D9: decimal point for upper display
 uint8_t upper_val_;  // D4-D1: upper display reading
 uint8_t upper_unit_; // D12-D11: annunciator for upper display unit
 uint8_t polarity1;
 uint8_t lowerdisplay_unit;
 uint8_t upperdisplay_unit;
 uint8_t lowerdisplay_dp;
 uint8_t upperdisplay_dp;
 uint16_t lowerdisplayread;
 uint16_t upperdisplayread;
};
class cDOSensor {
  private:
  bool isValidFrame;
  bool m_bFrameready;
  char m_FrameString[MAX_FRAME_SIZE];
  char sub[MAX_FRAME_SIZE];
  uint8_t pos;
  bool m_framestarted;
  uint8_t position;
  uint8_t length;
  uint8_t count;

  DO_frame m_doframe;

  public:
  CPondConfig m_oCpondCnfg;
  /*constructor*/
  cDOSensor();
  /*Destructor*/
  ~cDOSensor();
  uint8_t decode(char c);
  uint8_t ParseCommands(void);
  void substring(char [], char[], int, int);
  float m_ftemperature;
  float m_foxygen;
  String m_stempUnit;
  String m_soxygenUnit;
  uint8_t m_u8pondNum;
  char pondname[10];
  uint64_t tagId;

};

#endif // DISSOLVED_OXYGEN_METER_H

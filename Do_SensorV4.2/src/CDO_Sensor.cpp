/* 
  PM_APP.cpp - Application file is used to read the registor values of energy meter ic , calculate the suggested/ required capbank value and also used for detection of 
               power source
            
  Dev: Infiplus Team 
  May 2021  
*/

#include "CDO_Sensor.h"

//#define SERIAL_DEBUG
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

/* Constructor */
cDOSensor::cDOSensor()
{
  m_bFrameready = false;
}
/* Destructor */
cDOSensor::~cDOSensor(){}
/**************************************************
 * decode incomming data form DO sensor
 * Input : single char
 * output: result
**************************************************/
uint8_t cDOSensor::decode(char c)
{
  //debugPrint(c);
  switch(c)
  {
    case 0x02: // Start Of Frame
      {
        m_framestarted = true;
        isValidFrame = false;
        m_bFrameready = false;
        pos = 0;       
      }
      break;
    case 0x0D:
      {
        if(m_framestarted)
        {
          m_framestarted = false;
          m_FrameString[pos]= 0;
          isValidFrame = true;
        }
      }
      break;
    default:
      if(pos < MAX_FRAME_SIZE && m_framestarted)
        m_FrameString[pos++] = c;
      break;
  }
  if(isValidFrame)
  {
    isValidFrame = false;
    debugPrintln();
    debugPrintln(m_FrameString);
      for (int i = 0; i < pos; i++) {
      debugPrintf("%02X ", m_FrameString[i]);
    }
    debugPrintln();
    substring(m_FrameString,sub,1,1);
    uint8_t polarity1 = atoi(sub);            //polarity bit 0,1,2,3
    substring(m_FrameString,sub,2,1);
    uint8_t lowerdisplay_unit = atoi(sub);    //lower display and unit 1 = deg celsius 2 = deg Fahrenheit
    substring(m_FrameString,sub,3,2);
    uint8_t upperdisplay_unit = atoi(sub);    //upper display and unit 1 = deg C 2 = deg F 07 = mg/L 06 = %O2
    substring(m_FrameString,sub,5,1);
    uint8_t lowerdisplay_dp = atoi(sub);      //lower display and decimal point 
    substring(m_FrameString,sub,6,1);
    uint8_t upperdisplay_dp = atoi(sub);      // upper display and decimal point
    substring(m_FrameString,sub,7,4);
    uint16_t lowerdisplayread = atoi(sub);     //lower display reading D8 to D5
    substring(m_FrameString,sub,11,4);
    uint16_t upperdisplayread = atoi(sub);     //upper display reading D4 to D1

    debugPrintln(polarity1);
    debugPrintln(lowerdisplay_unit);
    debugPrintln(upperdisplay_unit);
    debugPrintln(lowerdisplay_dp);
    debugPrintln(upperdisplay_dp);
    debugPrintln(lowerdisplayread);
    debugPrintln(upperdisplayread);



      // Switch case for m_ftemperature unit
    switch(lowerdisplay_unit)
    {
    case 1:
    m_stempUnit = " C";
    debugPrintln("Unit: Degree Celsius");
    break;
    case 2:
    m_stempUnit = " F";
    debugPrintln("Unit: Degree Fahrenheit");
    break;
    default:
    m_stempUnit = "Unknown";
    debugPrintln("Unit: Unknown");
    break;
    }

    // Switch case for oxygen unit
    switch(upperdisplay_unit)
   {
    case 6:
    m_soxygenUnit = " %O2";
    debugPrintln("Unit: %O2");
    break;
    case 7:
    m_soxygenUnit = " mg/L";
    debugPrintln("Unit: mg/L");
    break;
    default:
    m_soxygenUnit = "Unknown";
    debugPrintln("Unit: Unknown");
    break;
}
  switch(polarity1)
   {
   case 0:
      debugPrint("Both Upper and Lower display value are '+' ");
      break;
      case 1:
      debugPrint("Upper '-' and Lower '+' ");
      break;
      case 2:
      debugPrint("Upper '+' lower '-' ");
      break;
      case 3:
      debugPrint("Both Upper and Lower display value are '+' ");
      break;
    }

    debugPrintln();
    if(polarity1 == 2){
    m_ftemperature = (lowerdisplayread/10.0)*-1;
    debugPrintf("temperature is: %.1f",m_ftemperature);
    }
    else if(polarity1 == 0){
    m_ftemperature = lowerdisplayread/10.0;
    debugPrintf("temperature is: %.1f",m_ftemperature);
    }
    else if(polarity1 == 1){
    m_ftemperature = lowerdisplayread/10.0;
    debugPrintf("temperature is: %.1f",m_ftemperature);
    }
    else if(polarity1 == 3){
    m_ftemperature = (lowerdisplayread/10.0)*-1;
    debugPrintf("temperature is: %.1f",m_ftemperature);
    }
    debugPrintln(m_stempUnit);
    debugPrintln();
    m_foxygen =  upperdisplayread/10.0;
    debugPrintf("oxygen : %f",m_foxygen);
    debugPrintln(m_soxygenUnit);
    delay(500);
    m_bFrameready = false;
  }
  return 0;
}
/**************************************************
 * Parse Commands 
 * Input : single char
 * output: result
**************************************************/
uint8_t cDOSensor::ParseCommands(void)
{
  /*Parse Command */
  if(m_bFrameready)
  {
    debugPrintln("Valid Frame...");
    // /*validation*/
    if(m_doframe.polarity_)
    {
      m_doframe.upper_val_ = m_doframe.upper_val_ - 1;
      
      debugPrintln(m_doframe.polarity_, HEX);
      debugPrintln(m_doframe.lower_dp_, HEX);
      debugPrintln(m_doframe.lower_val_, HEX);
      debugPrintln(m_doframe.lower_unit_, HEX);
      debugPrintln(m_doframe.upper_dp_, HEX);
      debugPrintln(m_doframe.upper_val_, HEX);
      debugPrintln(m_doframe.upper_unit_, HEX);
    
    }
    else{
      m_bFrameready = false;
      return 1; // Failure
    }
  }
  return 0; // No frame ready
}
// C substring function definition
 void cDOSensor::substring(char s[], char sub[], int p, int l) {
   int c = 0;
   
   while (c < l) {
      sub[c] = s[p+c-1];
      c++;
   }
   sub[c] = '\0';  
}
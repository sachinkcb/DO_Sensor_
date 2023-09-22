#ifndef DISPLAY_h
#define DISPLAY_h
#include <Arduino.h>


class cMYDISPLAY
	{
    private:		
      int m_i_clk_pin;
      int m_i_latch_pin;
      int m_i_data_pin;
      uint8_t m_igloble_buff[5];
      uint8_t m_iupdate_digit[5];// = {0x81};
      void update(uint8_t *m_iupdate_digit);
      uint8_t encodeDigit(uint8_t digit);
      uint8_t encodeChar(char ch);
      void setDisplayBuff(uint8_t *in_buff,uint8_t pos,uint8_t len);

    public:
      /* Construct */
      cMYDISPLAY(void);
      /* Destruct */
      ~cMYDISPLAY(void);

      /* Initialization Functions */	
      void init(int data_pin,int clk_pin,int latch_pin);
      void printString(uint8_t *disp_buff, uint8_t s_pos,uint8_t Length);
      void printStringNum(uint8_t *disp_buff, uint32_t Num, uint8_t s_pos,uint8_t Length);
      void printInt(uint32_t Num , uint8_t s_pos, uint8_t Length);
      void clear(uint8_t c_pos, uint8_t len);
      void printFloat(float Float_val,uint8_t s_pos,uint8_t Length,uint8_t decimals);
	};
#endif

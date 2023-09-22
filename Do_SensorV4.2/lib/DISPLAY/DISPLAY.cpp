#include "DISPLAY.h"
#include "DisplayLookUpTable.h"
#include <stdint.h>


/*Construct*/
cMYDISPLAY::cMYDISPLAY(){
  
}

/*Destruct*/
cMYDISPLAY::~cMYDISPLAY() {
	// end 
}

/********************************************************************
 * send serial data to one or more shift registers by 
 * iterating backwards through an array.
 * @param[in] m_iupdate_digit : Array of display daya
 ********************************************************************/
void cMYDISPLAY::update(uint8_t *m_iupdate_digit)
{
  uint8_t value=0;
  for (uint8_t reg = 5; reg >0; reg--)
  {
    value = m_iupdate_digit[reg-1];
    
    for (uint8_t bitMask = 128; bitMask > 0; bitMask >>= 1)
    {
      if((value & bitMask))
        digitalWrite(m_i_data_pin,1);   
      else
        digitalWrite(m_i_data_pin,0);       
      
       digitalWrite(m_i_clk_pin,0); 
       digitalWrite(m_i_clk_pin,1);    
    }
  }
  digitalWrite(m_i_latch_pin,0);  
  digitalWrite(m_i_latch_pin,1);    
}
/********************************************************************
 *@brief        Function for Display correct digits.
 *
 *@param[in]   digit  digit for display (0 ~ 9).
 ********************************************************************/
uint8_t cMYDISPLAY::encodeDigit(uint8_t digit)
{
  return m_ig_digits[digit & 0xff];
}

/********************************************************************
 *@brief        Function for Display correct characters.
 *
 *@param[in]   digit  digit for display (A,b,C,d,E,F,L,P,-,_,G,n,V,r,t,o,u)
           A
         -----
       F | G | B
         -----
       E |   | C
         -----   .H
           D
   _______________________________________________
  |  H  |  G  |  F  |  E  |  D  |  C  |  B  |  A  |
  |_____|_____|_____|_____|_____|_____|_____|_____|
  |  h  |  c  |  d  |  e  |  f  |  a  |  b  |  g  |
  |_____|_____|_____|_____|_____|_____|_____|_____|

 ********************************************************************/
/********************************************************************
 *@brief        Function for Display correct characters.
 *
 *@param[in]   digit  digit for display (A,b,C,d,E,F,L,P,-,_,G,n,V,r,t,o,u)
 ********************************************************************/
uint8_t cMYDISPLAY::encodeChar(char ch)
{ 
  // uint8_t num;
  switch(ch)
  {
      case ' ': return 0xff;
              break;
      case '.': return 0x7f;
              break;
      case 'A': return m_ig_char[0 & 0xff];
              break;
      case 'b': return m_ig_char[1 & 0xff];
              break;
      case 'C': return m_ig_char[2 & 0xff];
              break;
      case 'd': return m_ig_char[3 & 0xff];
              break;
      case 'E': return m_ig_char[4 & 0xff];
              break;
      case 'F': return m_ig_char[5 & 0xff];
              break;
      case 'G': return m_ig_char[10 & 0xff];
              break;
      case 'H': return m_ig_char[17 & 0xff];
              break;
      case 'L': return m_ig_char[6 & 0xff];
              break;
      case 'N': return m_ig_char[11 & 0xff];
              break;
      case 'V': return m_ig_char[12 & 0xff];
              break;
      case 'r': return m_ig_char[13 & 0xff];
              break;
      case 't':     return m_ig_char[14 & 0xff];
              break;
      case 'o':     return m_ig_char[15 & 0xff];
              break;
      case 'u':     return m_ig_char[16 & 0xff];
              break;
      case 'P': return m_ig_char[7 & 0xff];
              break;
      case 'S': return m_ig_digits[5 & 0xff];
              break;              
      case '-': return m_ig_char[8 & 0xff];
              break;
      case '0': return m_ig_digits[0 & 0xff];
              break;
      case '1': return m_ig_digits[1 & 0xff];
              break;
      case '2': return m_ig_digits[2 & 0xff];
              break;
      case '3': return m_ig_digits[3 & 0xff];
              break;
      case '4': return m_ig_digits[4 & 0xff];
              break;
      case '5': return m_ig_digits[5 & 0xff];
              break;
      case '6': return m_ig_digits[6 & 0xff];
              break;
      case '7': return m_ig_digits[7 & 0xff];
              break;  
      case '8': return m_ig_digits[8 & 0xff];
              break;
      case '9': return m_ig_digits[9 & 0xff];
              break;  
      default:
              break;
  }
  return 0xff;
}



/********************************************************************
 *@brief    Function for set the buffer position as per give 
 *      segment direction.
 *@param[in]   in_buff  : Input buffer to set the exact position as 
 *        per segment direction.
 *@param[in]   f_pos    : starting position of value
 ********************************************************************/
void cMYDISPLAY::setDisplayBuff(uint8_t *in_buff,uint8_t pos,uint8_t len)
{ 
  if(pos+len <= 5) 
  {
    for(uint8_t i=pos; i<pos+len; i++)
    {
      m_igloble_buff[m_iposition_buff[i]] = in_buff[i-pos];
    }
  }
  else
  {
    for(uint8_t i=0; i < 5; i++)
    {
      m_igloble_buff[m_iposition_buff[i]] = '-';
    }
  }
}

/********************************************************************
 *@brief       Function for Display value.
 *@param[in]   disp_buff  : charecter value.
 *@param[in]   s_pos      : starting position of value
 ********************************************************************/
void cMYDISPLAY::printString(uint8_t *disp_buff, uint8_t s_pos,uint8_t Length)
{  
  setDisplayBuff(disp_buff,s_pos,Length);
  
  for(uint8_t i=0; i<5; i++)
  {
    m_iupdate_digit[i]=encodeChar(m_igloble_buff[i]);
  }
  update(m_iupdate_digit);
}

void cMYDISPLAY::printStringNum(uint8_t *disp_buff, uint32_t Num, uint8_t s_pos,uint8_t Length)
{
  uint8_t buff[8] = "0";
  if(Num < 9)
    sprintf((char *)buff,"00%d%s",Num,(char *)disp_buff);
  else if(Num < 99)
    sprintf((char *)buff,"0%d%s",Num,(char *)disp_buff);
  else
    sprintf((char *)buff,"%d%s",Num,(char *)disp_buff);
  printString(buff,s_pos,Length);
}

/********************************************************************
 *@brief        Function for Display value.
 *
 *@param[in]   disp_buff  : charecter value.
 *@param[in]   s_pos      : starting position of value
 ********************************************************************/
void cMYDISPLAY::printInt(uint32_t Num , uint8_t s_pos, uint8_t length)
{
  uint8_t buff[20] = "0";

  if(length == 2)
    sprintf((char *)buff,"%02d",Num);
  else if(length == 3)
    sprintf((char *)buff,"%03d",Num);
  else
    sprintf((char *)buff,"%05d",Num);

  setDisplayBuff(buff,s_pos,length);

  for(uint8_t i=0; i<5; i++)
  {
    m_iupdate_digit[i]=encodeChar(m_igloble_buff[i]);
  }
  update(m_iupdate_digit);
}

/********************************************************************
 *@brief     Function for Clear the display segment.
 *
 *@param[in]   c_pos  : position of segment to clear.
 *@param[in]   len    : no. of segments to clear from @f_pos
 ********************************************************************/
void cMYDISPLAY::clear(uint8_t c_pos, uint8_t len)
{ 
  uint8_t clr_val = 0xff;   /*Hex value for clr display for Common Anode*/
  for(uint8_t i=c_pos;i<c_pos+len;i++)
  {
    m_iupdate_digit[m_iposition_buff[i]] = clr_val;
  }
 update(m_iupdate_digit);
}
/********************************************************************
 *@brief  Function for Display float value.
 *
 *@param[in]   before_dot   Integer value Before decimal point.
 *@param[in]   after_dot    Integer value After decimal point.
 ********************************************************************/
void cMYDISPLAY::printFloat(float Float_val,uint8_t s_pos,uint8_t length,uint8_t decimals)
{
  uint8_t buff[20] = "0";
  int dotPos = -1;
  int isdotfound = 0;
  
  if(decimals == 2) 
    sprintf((char *)buff,"%0.2f",Float_val);
  else
    sprintf((char *)buff,"%0.1f",Float_val);

  for(uint8_t i = 0; i < 19; i++)
  {
    if(buff[i] == '.') 
    {
      dotPos = i - 1;
      isdotfound = 1;
    }
    if(isdotfound)
    {
      buff[i] = buff[i + 1];
    }
  }
  setDisplayBuff(buff,s_pos,length);
   
  for(uint8_t i=0; i<5; i++)
  {
    m_iupdate_digit[i]=encodeChar(m_igloble_buff[i]);
  }

  if((dotPos != -1) && (dotPos < length))
  {
    m_iupdate_digit[dotPos] = m_iupdate_digit[dotPos] & 0x7F;
  }

  update(m_iupdate_digit);
}

/********************************************************************
 *@brief     Function for initializing display module *
 *@param[in]   data_pin   : GPIO pin number for display data pin
 *@param[in]   clk_pin    : GPIO pin number for display clock pin
 *@param[in]  latch_pin : GPIO pin number for display latch pin
 ********************************************************************/
void cMYDISPLAY::init(int data_pin,int clk_pin,int latch_pin)
{
  m_i_data_pin = data_pin;
  m_i_clk_pin = clk_pin;
  m_i_latch_pin = latch_pin;

  pinMode(m_i_latch_pin, OUTPUT);
  pinMode(m_i_clk_pin, OUTPUT);
  pinMode(m_i_data_pin, OUTPUT);

  digitalWrite(m_i_latch_pin, 0);
  digitalWrite(m_i_clk_pin, 0);
  digitalWrite(m_i_data_pin, 0);
}

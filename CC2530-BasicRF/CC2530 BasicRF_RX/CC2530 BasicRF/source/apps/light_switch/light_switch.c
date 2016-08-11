
#include <hal_lcd.h>
#include <hal_led.h>
#include <hal_joystick.h>
#include <hal_assert.h>
#include <hal_board.h>
#include <hal_int.h>
#include "hal_mcu.h"
#include "hal_button.h"
#include "hal_rf.h"
#include "util_lcd.h"
#include "basic_rf.h"
#include <string.h>
#include <ioCC2530.h>
/***********************************************************************************
* CONSTANTS
*/
// Application parameters
#define RF_CHANNEL                25      // 2.4 GHz RF channel

// BasicRF address definitions
#define PAN_ID                0x2007
#define SWITCH_ADDR           0x2520
#define LIGHT_ADDR            0xBEEF
#define APP_PAYLOAD_LENGTH        1
#define LIGHT_TOGGLE_CMD          0

// Application states
#define IDLE                      0
#define SEND_CMD                  1

// Application role
#define NONE                      0
#define SWITCH                    1
#define LIGHT                     2
#define APP_MODES                 2


static uint8 pRxData[APP_PAYLOAD_LENGTH];
static basicRfCfg_t basicRfConfig;

void initUARTtest(void);
void UartTX_Send_String(unsigned char *Data);

static void appLight();
unsigned char string1[] = { "AT+CSMP=17,167,2,25\r\n"};
unsigned char string2[] = { "at+cmgf=1\r\n"};       //文本模式，0--pdu模式
unsigned char string3[] = { "at+cscs=\"GSM\"\r\n"};
unsigned char string4[] = { "at+cmgs=\"12345678907\"\r\n"}; //电话号码
unsigned char string6[] = { "sos\n" };
unsigned char string7[] = { "\x1a\r\n" };


void _delay_cycles(unsigned long x)
    {
    while(x--);
    }

static void appLight()
{
  /*************************************************by boo
    halLcdWriteLine(HAL_LCD_LINE_1, "Light");
    halLcdWriteLine(HAL_LCD_LINE_2, "Ready");
  */  
#ifdef ASSY_EXP4618_CC2420
    halLcdClearLine(1);
    halLcdWriteSymbol(HAL_LCD_SYMBOL_RX, 1);
#endif

    // Initialize BasicRF
    basicRfConfig.myAddr = LIGHT_ADDR;
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }
    basicRfReceiveOn();

    // Main loop
    while (TRUE) {
        while(!basicRfPacketIsReady());

        if(basicRfReceive(pRxData, APP_PAYLOAD_LENGTH, NULL)>0) {
            if(pRxData[0] == LIGHT_TOGGLE_CMD) 
            {
              P1_1=~P1_1;
          //    for(int i=0;i<2;i++)
            //      {
                    UartTX_Send_String(string2);
                    _delay_cycles(50000);

                    UartTX_Send_String(string3);
                    _delay_cycles(50000);
                    
                    UartTX_Send_String(string1);
                    _delay_cycles(50000);

                    UartTX_Send_String(string4);
                    _delay_cycles(50000);

                    UartTX_Send_String(string6);
                    _delay_cycles(50000);

                    UartTX_Send_String(string7);
                    _delay_cycles(100000);
           //        }
              P1_0=~P1_0;
            }
        }
    }
}


void main(void)
{
    // Config basicRF
    basicRfConfig.panId = PAN_ID;
    basicRfConfig.channel = RF_CHANNEL;
    basicRfConfig.ackRequest = TRUE;
#ifdef SECURITY_CCM
    basicRfConfig.securityKey = key;
#endif

    // Initalise board peripherals
    halBoardInit();                             // Initalise hal_rf
  
  initUARTtest();                         //初始化串口 

    if(halRfInit()==FAILED) {
      HAL_ASSERT(FALSE);
    }

    // Indicate that device is powered
    halLedSet(2);//*****************by boo LED2(P1_1=1)
    halLedClear(1);//***************by boo LED1(P1_0=0)


  
   appLight();       //节点为指示灯LED1   P1_0  
    // Role is undefined. This code should not be reached
    HAL_ASSERT(FALSE);
}


void initUARTtest(void)
{
  
    SLEEPCMD&= ~0X04;
    CLKCONCMD = 0X10;         //选择32KHz晶体振荡器,32MHz晶体振荡器,定时器tick输出8MHz
    while(CLKCONSTA!=0X10);   //晶振是否稳定
    SLEEPCMD = 0X04;    

    PERCFG = 0x00;				//位置1 P0口
    P0SEL = 0x3c;				//P0用作串口
    P2DIR &= ~0XC0;                             //P0优先作为串口0

    U0CSR |= 0x80;				//UART方式
    U0GCR |= 10;				//baud_e
    U0BAUD |= 216;				//波特率设为57600
    UTX0IF = 0;
}

void UartTX_Send_String(unsigned char *Data)
{
  
  while(*Data!='\n')
  {
    U0DBUF = *Data++;
    while(UTX0IF == 0);
    UTX0IF = 0;
  }
}

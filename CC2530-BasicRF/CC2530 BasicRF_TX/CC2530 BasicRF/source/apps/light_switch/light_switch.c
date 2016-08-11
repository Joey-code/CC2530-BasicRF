
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

/***********************************************************************************
* LOCAL VARIABLES
*/
static uint8 pTxData[APP_PAYLOAD_LENGTH];
static uint8 pRxData[APP_PAYLOAD_LENGTH];
static basicRfCfg_t basicRfConfig;


static void appSwitch();

static void appSwitch()
{
/****************************************************************by boo  
  halLcdWriteLine(HAL_LCD_LINE_1, "Switch");
    halLcdWriteLine(HAL_LCD_LINE_2, "Joystick Push");
    halLcdWriteLine(HAL_LCD_LINE_3, "Send Command");*/
#ifdef ASSY_EXP4618_CC2420
    halLcdClearLine(1);
    halLcdWriteSymbol(HAL_LCD_SYMBOL_TX, 1);
#endif


    // Initialize BasicRF
    basicRfConfig.myAddr = SWITCH_ADDR;
    if(basicRfInit(&basicRfConfig)==FAILED) {
      HAL_ASSERT(FALSE);
    }
    
    pTxData[0] = LIGHT_TOGGLE_CMD;

    // Keep Receiver off when not needed to save power
    basicRfReceiveOff();

    // Main loop
    while (TRUE) {
        //if( halJoystickPushed() )**********************by boo
      if(halButtonPushed()==HAL_BUTTON_1)//**************by boo
        {

            basicRfSendPacket(LIGHT_ADDR, pTxData, APP_PAYLOAD_LENGTH);
            P1_1=~P1_1;
            // Put MCU to sleep. It will wake up on joystick interrupt
            halIntOff();
            halMcuSetLowPowerMode(HAL_MCU_LPM_3); // Will turn on global
            // interrupt enable
            halIntOn();

        }
    }
}


/***********************************************************************************
* @fn          main
*
* @brief       This is the main entry of the "Light Switch" application.
*              After the application modes are chosen the switch can
*              send toggle commands to a light device.
*
* @param       basicRfConfig - file scope variable. Basic RF configuration
*              data
*              appState - file scope variable. Holds application state
*
* @return      none
*/
void main(void)
{
    uint8 appMode = NONE;

    // Config basicRF
    basicRfConfig.panId = PAN_ID;
    basicRfConfig.channel = RF_CHANNEL;
    basicRfConfig.ackRequest = TRUE;
#ifdef SECURITY_CCM
    basicRfConfig.securityKey = key;
#endif

    // Initalise board peripherals
    halBoardInit();
 //   halJoystickInit();

    // Initalise hal_rf
    if(halRfInit()==FAILED) {
      HAL_ASSERT(FALSE);
    }

    // Indicate that device is powered
    halLedSet(2);//*****************by boo LED2(P1_1=1)
    halLedClear(1);//***************by boo LED1(P1_0=0)
    
    /************Select one and shield to another***********by boo*/
    appSwitch();        //节点为按键S1       P0_4    
    // Role is undefined. This code should not be reached
    HAL_ASSERT(FALSE);
}


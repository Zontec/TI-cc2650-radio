
#include <xdc/std.h>
#include <xdc/runtime/System.h>

/* BIOS Header files */
#include <ti/sysbios/BIOS.h>
#include <ti/sysbios/knl/Task.h>
#include <ti/sysbios/knl/Clock.h>

/* TI-RTOS Header files */
#include <ti/drivers/I2C.h>
#include <ti/drivers/PIN.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/UART.h>
#include <ti/drivers/Watchdog.h>

/* Radio libs*/
#include <ti/drivers/rf/RF.h>
#include <driverlib/rf_ieee_cmd.h>
#include <driverlib/rfc.h>
#include "smartrf_settings/smartrf_settings.h"

/* Board Header files */
#include "Board.h"
#include "Serial.h"
#include "RFQueue.h"
#define TASKSTACKSIZE   4096

Task_Struct task0Struct;
Char task0Stack[TASKSTACKSIZE];

Task_Struct task1Struct;
Char task1Stack[TASKSTACKSIZE];

/* Pin driver handle */
static PIN_Handle ledPinHandle;
static PIN_State ledPinState;

static RF_Object    rfObject;
static RF_Handle    rfHandle;


/*
 * Application LED pin configuration table:
 *   - All LEDs board LEDs are off.
 */
PIN_Config ledPinTable[] = {
    Board_LED0 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    Board_LED1 | PIN_GPIO_OUTPUT_EN | PIN_GPIO_LOW | PIN_PUSHPULL | PIN_DRVSTR_MAX,
    PIN_TERMINATE
};

#define GREEN_LED                   IOID_7
#define RED_LED                     IOID_6

#define GREEN_LED_UP                PIN_setOutputValue(ledPinHandle, GREEN_LED, 1)
#define GREEN_LED_DOWN              PIN_setOutputValue(ledPinHandle, GREEN_LED, 0)

#define RED_LED_UP                  PIN_setOutputValue(ledPinHandle, RED_LED, 1)
#define RED_LED_DOWN                PIN_setOutputValue(ledPinHandle, RED_LED, 0)

#define RED_LED_TOGGLE              PIN_setOutputValue(ledPinHandle, RED_LED, !PIN_getOutputValue(RED_LED))
#define GREEN_LED_TOGGLE            PIN_setOutputValue(ledPinHandle, GREEN_LED, !PIN_getOutputValue(GREEN_LED))

#define DATA_ENTRY_HEADER_SIZE 8  /*  Generic Data Entry */
#define MAX_LENGTH             30 /* Max length byte the radio will accept */
#define NUM_DATA_ENTRIES       2  /* Number of data entries */

#define NUM_APPENDED_BYTES     2

#pragma DATA_ALIGN (rxDataEntryBuffer, 4);
        static uint8_t rxDataEntryBuffer[RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(NUM_DATA_ENTRIES,
                                                                 MAX_LENGTH,
                                                                 NUM_APPENDED_BYTES)];

/*
 *  ======== heartBeatFxn ========
 *  Toggle the Board_LED0. The Task_sleep is determined by arg0 which
 *  is configured for the heartBeat Task instance.
 */

enum RFMode{
    Recive=0,
    Transmite=1,
};

static dataQueue_t dataQueue;
static rfc_dataEntryGeneral_t* currentDataEntry;
static uint8_t packetLength;
static uint8_t* packetDataPointer;

void RF_mode(enum RFMode mode){
    RF_cmdFs.synthConf.bTxMode = mode;
    RF_runCmd(rfHandle, (RF_Op*)&RF_cmdFs, RF_PriorityNormal, NULL, 0);
}

void txcallback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e){

    RF_cancelCmd(h, ch, 0);
    GREEN_LED_TOGGLE;
}

void transmiteIEEE802_15_4(char *buf, unsigned int size) {

    /*Set TX mode*/
    RF_mode(Transmite);

    RF_cmdIeeeTx.payloadLen = size;
    RF_cmdIeeeTx.pPayload = buf;
    RF_cmdIeeeTx.startTrigger.triggerType = TRIG_NOW;

    RF_runCmd(rfHandle, (RF_Op*)&RF_cmdIeeeTx, RF_PriorityHigh, &txcallback, IRQ_TX_DONE);
    //RFCDoorbellSendTo(&RF_cmdIeeeTx);
    _printf("done");
    __delay(10);
}
int dataLen= 0;

void __delay(long int ms){
    for(int i = 0; i < ms * 1000; i++)
        asm("nop;");
}

void callback(RF_Handle h, RF_CmdHandle ch, RF_EventMask e)
{

    if (e & RF_EventRxEntryDone)
    {
        GREEN_LED_TOGGLE;
        currentDataEntry = RFQueue_getDataEntry();

        packetDataPointer = (uint8_t *)(&currentDataEntry->data);

        dataLen = currentDataEntry->length;
        switch(packetDataPointer[4])
        {
            case '1':
                RED_LED_UP;
                break;
            case '2':
                RED_LED_DOWN;
                break;
            case '3':
                GREEN_LED_UP;
                break;

            case '4':
                GREEN_LED_DOWN;
                break;
            default:
                break;
        }

        /* Copy the payload + the status byte to the packet variable */
     //   memcpy(packet, packetDataPointer, (packetLength + 1));
        RFQueue_nextEntry();
    }
}

void secondThread(UArg arg0, UArg arg1) {

    while(1){
            RED_LED_TOGGLE;
            Task_sleep((UInt)arg0);
    }
}

void mainThread(UArg arg0, UArg arg1) {

    char buf[] = "xxHello";
    initDefaultUart();
    RF_Params rfParams;
    RF_Params_init(&rfParams);

    rfHandle = RF_open(&rfObject, &RF_prop, (RF_RadioSetup*)&RF_cmdRadioSetup, &rfParams);
    RF_mode(Recive);

    if( RFQueue_defineQueue(&dataQueue,
                            rxDataEntryBuffer,
                            sizeof(rxDataEntryBuffer),
                            NUM_DATA_ENTRIES,
                            MAX_LENGTH + NUM_APPENDED_BYTES))
    {
        while(1);
    }

    RF_cmdIeeeRx.pRxQ = &dataQueue;
    _printf("Hello\r\n");
    RF_CmdHandle RF_CmdHandle;
    while(1){


        if(RF_cmdIeeeRx.status != 2)
            RF_CmdHandle = RF_postCmd(rfHandle, (RF_Op*)&RF_cmdIeeeRx, RF_PriorityNormal, &callback, IRQ_RX_ENTRY_DONE);
        Task_sleep((UInt)arg0);

        RF_cancelCmd(rfHandle, RF_CmdHandle, 1);

        _printf("Recived: %d\r\n", dataLen);
        dataLen = 0;

        transmiteIEEE802_15_4(buf, sizeof(buf));
        RF_mode(Recive);
        Task_sleep((UInt)arg0);


    }
}




/*
 *  ======== main ========
 */
int main(void)
{
    Task_Params taskParams;

    UART_init();
    Board_initGeneral();

    /* Open LED pins */
    ledPinHandle = PIN_open(&ledPinState, ledPinTable);
    if(!ledPinHandle) {
        System_abort("Error initializing board LED pins\n");
    }

    /*    First thread     */

    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000000 / Clock_tickPeriod;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task0Stack;
    taskParams.priority = 2;

    Task_construct(&task0Struct, (Task_FuncPtr)mainThread, &taskParams, NULL);


    /*    Second thread     */

    Task_Params_init(&taskParams);
    taskParams.arg0 = 1000000 / Clock_tickPeriod;
    taskParams.stackSize = TASKSTACKSIZE;
    taskParams.stack = &task1Stack;
    taskParams.priority = 2;

    Task_construct(&task1Struct, (Task_FuncPtr)secondThread, &taskParams, NULL);


    /* Start BIOS */
    BIOS_start();

    return (0);
}

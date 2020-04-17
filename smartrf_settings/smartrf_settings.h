#ifndef _SMARTRF_SETTINGS_H_
#define _SMARTRF_SETTINGS_H_


#include <driverlib/rf_mailbox.h>
#include <driverlib/rf_common_cmd.h>
#include <driverlib/rf_ieee_cmd.h>
#include <ti/drivers/rf/RF.h>



extern RF_Mode RF_prop;

//Look if volatile needed
extern rfc_CMD_RADIO_SETUP_t RF_cmdRadioSetup;
extern rfc_CMD_FS_t RF_cmdFs;
extern rfc_CMD_IEEE_RX_t RF_cmdIeeeRx;
extern rfc_CMD_IEEE_TX_t RF_cmdIeeeTx;


extern uint32_t pOverrides[];

#endif 

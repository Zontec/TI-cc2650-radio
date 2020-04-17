/*
    Date: 17.10.2019
    Created by: Zontec
    All rights reserved


*/

/*
 * This module provides with main RF functions
 * */


#include <stdint.h>
#include <stdlib.h>

#include "RFQueue.h"
#include <driverlib/rf_data_entry.h>


/* Receive entry pointer to keep track of read items */
rfc_dataEntryGeneral_t* readEntry;

rfc_dataEntryGeneral_t* RFQueue_getDataEntry()
{
  return (readEntry);
}


uint8_t RFQueue_nextEntry()
{
  readEntry -> status = DATA_ENTRY_PENDING;

  readEntry = (rfc_dataEntryGeneral_t*)readEntry -> pNextEntry;

  return readEntry -> status;
}

/*!!!!!!!!!!!!!!!!!!!!!*/
/* !!!!!!!!!!!!!!!!!!!!!!!!!! Not sure if it works OK  !!!!!!!!!!!!!!!!!!!!!!!!! */
/*!!!!!!!!!!!!!!!!!!!!!*/
uint8_t RFQueue_defineQueue(dataQueue_t *dataQueue, uint8_t *buf,
                            uint16_t buf_len, uint8_t numEntries, uint16_t length)
{

  if (buf_len < (numEntries * (length + RF_QUEUE_DATA_ENTRY_HEADER_SIZE + RF_QUEUE_QUEUE_ALIGN_PADDING(length))))
      return 1;

  /* Some error can happen here! Works with cc2560!*/
  uint8_t pad = 4 - ((length + RF_QUEUE_DATA_ENTRY_HEADER_SIZE) % 4);

  /* Set the Data Entries common configuration */
  uint8_t *first_entry = buf;
  for (int i = 0; i < numEntries; i++)
  {
    buf = first_entry + i * (RF_QUEUE_DATA_ENTRY_HEADER_SIZE + length + pad);
    /*Pending - starting state*/
    ((rfc_dataEntry_t*)buf) -> status        = DATA_ENTRY_PENDING;
    /* General Data Entry*/
    ((rfc_dataEntry_t*)buf) -> config.type   = DATA_ENTRY_TYPE_GEN;
    ((rfc_dataEntry_t*)buf) -> config.lenSz  = 0;
    ((rfc_dataEntry_t*)buf) -> length        = length;

    ((rfc_dataEntryGeneral_t*)buf) -> pNextEntry =
                                        &(((rfc_dataEntryGeneral_t*)buf) -> data) + length + pad;
  }
  ((rfc_dataEntry_t*)buf) -> pNextEntry = first_entry;

  dataQueue->pCurrEntry = first_entry;
  dataQueue->pLastEntry = NULL;


  readEntry = (rfc_dataEntryGeneral_t*) first_entry;

  return 0;
}

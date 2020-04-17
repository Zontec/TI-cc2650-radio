/*
    Date: 17.10.2019
    Created by: Zontec
    All rights reserved


*/

/*
 * This module provides with main RF functions
 * */


#ifndef RF_QUEUE_H
#define RF_QUEUE_H

#include <driverlib/rf_data_entry.h>

#define RF_QUEUE_DATA_ENTRY_HEADER_SIZE  8 // Contant header size of a Generic Data Entry

#define RF_QUEUE_QUEUE_ALIGN_PADDING(length)    \
                                        (4-((length + RF_QUEUE_DATA_ENTRY_HEADER_SIZE) % 4)) //

#define RF_QUEUE_DATA_ENTRY_BUFFER_SIZE(numEntries, dataSize, appendedBytes)                  \
                    (numEntries*(RF_QUEUE_DATA_ENTRY_HEADER_SIZE + dataSize + appendedBytes + \
                    RF_QUEUE_QUEUE_ALIGN_PADDING(dataSize + appendedBytes)))

extern uint8_t RFQueue_nextEntry();
extern rfc_dataEntryGeneral_t* RFQueue_getDataEntry();
extern uint8_t RFQueue_defineQueue(dataQueue_t *queue ,uint8_t *buf, 
									uint16_t buf_len, uint8_t numEntries, uint16_t length);

#endif

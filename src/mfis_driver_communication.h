/**
 * \file mfis_driver_communication.h
 * \brief Header for communication with MFIS kernel driver
 * \author esoftthings
 */

#ifndef MFIS_DRIVER_COMMUNICATION_H
#define MFIS_DRIVER_COMMUNICATION_H

#include <stdint.h>

/******************************************************************************************
 * Public Definitions
 ******************************************************************************************/
/**  \def  MFIS_MSG_SIZE: number of 32bits word per MFIS request */
#define MFIS_MSG_SIZE 8

/******************************************************************************************
 * Public Functions Prototypes
 ******************************************************************************************/
int mfis_send_request(int32_t *send, int32_t *receive);
void *mfis_get_virtual_address(const uint32_t physical_address, uint32_t mem_size);
#endif /* MFIS_DRIVER_COMMUNICATION_H */

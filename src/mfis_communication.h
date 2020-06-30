/**
 * \file mfis_communication.h
 * \brief Header for communication with MFIS kernel driver
 * \author esoftthings
 */

#ifndef MFIS_COMMUNICATION_H
#define MFIS_COMMUNICATION_H

#include <stdint.h>
#include <linux/eviewitf-mfis.h>

/******************************************************************************************
 * Typedef definitions
 ******************************************************************************************/

/******************************************************************************************
 * Public Definitions
 ******************************************************************************************/

/******************************************************************************************
 * Public Functions Prototypes
 ******************************************************************************************/
int mfis_init();
int mfis_deinit();
int mfis_send_request(int32_t *send, int32_t *receive);
int mfis_get_cam_attributes(struct eviewitf_mfis_camera_attributes *cameras_attributes);
int mfis_get_blend_attributes(struct eviewitf_mfis_blending_attributes *blendings_attributes);
void *mfis_get_virtual_address(const uint32_t physical_address, uint32_t mem_size);
#endif /* MFIS_COMMUNICATION_H */

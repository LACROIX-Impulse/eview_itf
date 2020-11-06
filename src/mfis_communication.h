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
int mfis_send_request(int32_t *request);
int mfis_get_cam_attributes(struct eviewitf_mfis_camera_attributes *cameras_attributes);
int mfis_get_blend_attributes(struct eviewitf_mfis_blending_attributes *blendings_attributes);
int mfis_ioctl_request(uint8_t devtype, uint8_t devid, uint16_t cmd, void *param);

#endif /* MFIS_COMMUNICATION_H */

/**
 * \file eviewitf-priv.h
 * \brief Internal header for private structures
 * \author esoftthings
 */

#ifndef EVIEWITF_PRIV_H
#define EVIEWITF_PRIV_H

#include <stdint.h>
#include "mfis_communication.h"

/******************************************************************************************
 * Private Definitions
 ******************************************************************************************/

/******************************************************************************************
 * Private Structures
 ******************************************************************************************/
/* Structures or enum used for internal lib purpose.
 Doesn't need to be exposed in API */

/**
 * \enum fct_ret_r
 * \brief Return codes
 */
typedef enum {
    FCT_RETURN_OK = 1,
    FCT_RETURN_BLOCKED,
    FCT_INV_PARAM,
    FCT_RETURN_ERROR,
} fct_ret_r;

/******************************************************************************************
 * Private Functions Prototypes
 ******************************************************************************************/

/* Cameras */
int eviewitf_reboot_cam(int cam_id);
int eviewitf_record_cam(int cam_id, int delay);
int eviewitf_play_on_virtual_cam(int cam_id, int fps, char *frames_dir);
int eviewitf_set_blending_from_file(int blending_id, char *frame);
void eviewitf_retrieve_cam_attributes(mfis_camera_attributes **cameras_attributes, uint8_t *init);

#endif /* EVIEWITF_PRIV_H */

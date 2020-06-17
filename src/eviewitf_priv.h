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

static const char *mfis_device_filenames[EVIEWITF_MAX_CAMERA] = {
    "/dev/mfis_cam0",  "/dev/mfis_cam1",  "/dev/mfis_cam2",  "/dev/mfis_cam3", "/dev/mfis_cam4",  "/dev/mfis_cam5",
    "/dev/mfis_cam6",  "/dev/mfis_cam7",  "/dev/mfis_cam8",  "/dev/mfis_cam9", "/dev/mfis_cam10", "/dev/mfis_cam11",
    "/dev/mfis_cam12", "/dev/mfis_cam13", "/dev/mfis_cam14", "/dev/mfis_cam15"};

/******************************************************************************************
 * Private Functions Prototypes
 ******************************************************************************************/

/* Cameras */
int eviewitf_reboot_cam(int cam_id);
int eviewitf_record_cam(int cam_id, int delay);
int eviewitf_play_on_virtual_cam(int cam_id, int fps, char *frames_dir);
int eviewitf_set_blending_from_file(int blending_id, char *frame);
int eviewitf_is_initialized();
mfis_camera_attributes *eviewitf_get_camera_attributes(int cam_id);

#endif /* EVIEWITF_PRIV_H */

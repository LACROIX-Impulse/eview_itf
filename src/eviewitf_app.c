/**
 * \file
 * \brief Communication API between A53 and R7 CPUs
 * \author esoftthings
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mfis_communication.h"
#include "eviewitf_priv.h"
#include "eviewitf_ssd.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/

/******************************************************************************************
 * Private structures
 ******************************************************************************************/

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/**
 * \fn eviewitf_app_record_cam
 * \brief Request R7 to change camera used on display
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param delay: duration of the record in seconds
 * \return state of the function. Return 0 if okay
 */
int eviewitf_app_record_cam(int cam_id, int delay, char *record_path) {
    int ret = EVIEWITF_OK;
    char *record_dir = NULL;
    eviewitf_device_attributes_t attributes;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        printf("Invalid camera id\n");
        printf("Please choose a real camera for the record\n");
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        if (record_path == NULL) {
            eviewitf_ssd_get_output_directory(&record_dir);
        } else {
            record_dir = record_path;
        }
        printf("SSD storage directory %s \n", record_dir);
        eviewitf_camera_get_attributes(cam_id, &attributes);
        ret = eviewitf_ssd_record_stream(cam_id, delay, record_dir, attributes.buffer_size);
        if (record_path == NULL) {
            free(record_dir);
        }
    }
    return ret;
}

/**
 * \fn eviewitf_app_reset_camera
 * \brief Request R7 to reset camera, currently not exposed in libeviewitf
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return state of the function. Return 0 if okay
 */
int eviewitf_app_reset_camera(int cam_id) {
    int ret = EVIEWITF_OK;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        request[0] = EVIEWITF_MFIS_FCT_CAM_RESET;
        request[1] = cam_id;
        ret = mfis_send_request(request);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (request[0] != EVIEWITF_MFIS_FCT_CAM_RESET) {
                ret = EVIEWITF_FAIL;
            } else if (request[1] == EVIEWITF_MFIS_FCT_RETURN_ERROR) {
                ret = EVIEWITF_FAIL;
            } else if (request[1] == EVIEWITF_MFIS_FCT_INV_PARAM) {
                ret = EVIEWITF_INVALID_PARAM;
            }
        }
    }
    return ret;
}

/**
 * \fn eviewitf_app_streamer_play
 * \brief Update the frames to be printed on a streamer

 * \param in streamer_id: id of the streamer
 * \param in fps: fps to apply on the recording
 * \param in frames_dir: path to the recording
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_app_streamer_play(int streamer_id, int fps, char *frames_dir) {
    int ret = EVIEWITF_OK;

    /* Test API has been initialized */
    if (eviewitf_is_initialized() == 0) {
        ret = EVIEWITF_NOT_INITIALIZED;
    }

    if (EVIEWITF_OK == ret) {
        /* Test camera id */
        if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
            ret = EVIEWITF_INVALID_PARAM;
        }
    }

    if (EVIEWITF_OK == ret) {
        ret = eviewitf_ssd_streamer_play(
            streamer_id, (get_device_object(streamer_id + EVIEWITF_OFFSET_STREAMER))->attributes.buffer_size, fps,
            frames_dir);
    }

    return ret;
}

/**
 * \fn eviewitf_app_set_blending_from_file
 * \brief Set a blending frame

 * \param in frame: path to the blending frame
 * \return state of the function. Return 0 if okay
 */
int eviewitf_app_set_blending_from_file(int blender_id, char *frame) {
    int ret = EVIEWITF_OK;

    /* Test API has been initialized */
    if (eviewitf_is_initialized() == 0) {
        ret = EVIEWITF_NOT_INITIALIZED;
    }

    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (EVIEWITF_OK == ret) {
        ret = eviewitf_ssd_set_blending(
            blender_id, get_device_object(blender_id + EVIEWITF_OFFSET_BLENDER)->attributes.buffer_size, frame);
    }

    return ret;
}

int eviewitf_app_print_monitoring_info(void) {
    int ret = EVIEWITF_OK;
    uint32_t data[EVIEWITF_MONITORING_INFO_SIZE] = {0};

    ret = eviewitf_get_monitoring_info(data, EVIEWITF_MONITORING_INFO_SIZE);
    if (EVIEWITF_OK == ret) {
        for (int i = 0; i < EVIEWITF_MONITORING_INFO_SIZE; i++) {
            printf("Raw monitoring %d: 0x%08X\n", i, data[i]);
        }
    }

    return ret;
}
/**
 * @file eviewitf-app.c
 * @brief Communication API between A53 and R7 CPUs
 * @author LACROIX Impulse
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cam-ioctl.h"
#include "mfis-communication.h"
#include "mfis-ioctl.h"
#include "eviewitf-priv.h"
#include "eviewitf-ssd.h"

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
 * @fn eviewitf_ret_t eviewitf_app_record_cam(int cam_id, int delay, char *record_path)
 * @brief Request R7 to change camera used on display
 *
 * @param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * @param delay: duration of the record in seconds
 * @param record_path: record path
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_app_record_cam(int cam_id, int delay, char *record_path) {
    eviewitf_ret_t ret = EVIEWITF_OK;
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
 * @fn eviewitf_ret_t eviewitf_app_reset_camera(int cam_id)
 * @brief Request R7 to reset camera, currently not exposed in libeviewitf
 *
 * @param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_app_reset_camera(int cam_id) {
    eviewitf_ret_t ret = EVIEWITF_OK;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCCAMREBOOT, NULL);
    }
    return ret;
}

/**
 * @fn eviewitf_ret_t eviewitf_app_streamer_play(int streamer_id, int fps, char *frames_dir)
 * @brief Update the frames to be printed on a streamer

 * @param streamer_id: id of the streamer
 * @param fps: fps to apply on the recording
 * @param frames_dir: path to the recording
 *
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_app_streamer_play(int streamer_id, int fps, char *frames_dir) {
    eviewitf_ret_t ret = EVIEWITF_OK;

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
 * @fn eviewitf_ret_t eviewitf_app_set_blending_from_file(int blender_id, char *frame)
 * @brief Set a blending frame

 * @param blender_id: blender identifier
 * @param frame: path to the blending frame
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_app_set_blending_from_file(int blender_id, char *frame) {
    eviewitf_ret_t ret = EVIEWITF_OK;

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

/**
 * @fn eviewitf_ret_t eviewitf_app_print_monitoring_info(void)
 * @brief Print app monitoring information
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_app_print_monitoring_info(void) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    uint32_t data[EVIEWITF_MONITORING_INFO_SIZE] = {0};

    ret = eviewitf_get_monitoring_info(data, EVIEWITF_MONITORING_INFO_SIZE);
    if (EVIEWITF_OK == ret) {
        for (int i = 0; i < EVIEWITF_MONITORING_INFO_SIZE; i++) {
            printf("Raw monitoring %d: 0x%08X\n", i, data[i]);
        }
    }

    return ret;
}

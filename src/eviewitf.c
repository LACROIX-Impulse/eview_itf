/**
 * @file eviewitf.c
 * @brief Communication API between A53 and R7 CPUs
 * @author LACROIX Impulse
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "mfis-communication.h"
#include "eviewitf-priv.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/
/**
 * @brief Maximum version size
 */
#define MAX_VERSION_SIZE 21

/******************************************************************************************
 * Private structures
 ******************************************************************************************/

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/

/**
 * @brief Global initialization indicator
 */
static uint8_t eviewitf_global_init = 0;

/**
 * @brief eView version
 */
static char eview_version[MAX_VERSION_SIZE];

/**
 * @brief eView initialization mutex
 */
static pthread_mutex_t eviewitf_init_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief eView deinitialization mutex
 */
static pthread_mutex_t eviewitf_deinit_mutex = PTHREAD_MUTEX_INITIALIZER;

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/**
 * @fn eviewitf_is_initialized
 * @brief Check if initialization has been performed
 *
 * @return Return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_is_initialized() { return eviewitf_global_init; }

/**
 * @fn eviewitf_init
 * @brief Initialize the eViewItf API
 * @ingroup eview
 *
 * @return Return code as specified by the eviewitf_ret_t enumeration.
 *
 * Initialize the eViewItf API by opening a communication with eView and by retrieving devices information from eView.
 * This function must be called before any other function of this API.
 * Otherwise, the other functions will return the error code EVIEWITF_NOT_INITIALIZED (eviewitf_return_state).
 */
eviewitf_ret_t eviewitf_init(void) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Critical section */
    pthread_mutex_lock(&eviewitf_init_mutex);

    /* Check if init has been done */
    if (eviewitf_global_init != 0) {
        ret = EVIEWITF_ALREADY_INITIALIZED;
    } else {
        mfis_init();

        /* Prepare TX buffer */
        request[0] = EVIEWITF_MFIS_FCT_INIT;

        /* Send request to R7 and check returned answer state*/
        ret = mfis_send_request(request);
        if ((ret < EVIEWITF_OK) || (request[0] != EVIEWITF_MFIS_FCT_INIT) ||
            (request[1] != EVIEWITF_MFIS_FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
        }

        /* Devices initialization */
        if (ret == EVIEWITF_OK) {
            ret = device_objects_init();
        }

        if (ret == EVIEWITF_OK) {
            eviewitf_global_init = 1;
        }
    }

    /* End of critical section */
    pthread_mutex_unlock(&eviewitf_init_mutex);
    return ret;
}

/**
 * @fn eviewitf_deinit
 * @brief De-initialize the eViewItf API
 * @ingroup eview
 *
 * @return return code as specified by the eviewitf_ret_t enumeration.
 *
 * De-initialize the eViewItf API by closing the communication with eView.
 */
eviewitf_ret_t eviewitf_deinit(void) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Critical section */
    pthread_mutex_lock(&eviewitf_deinit_mutex);

    /* Check if init has been done */
    if (eviewitf_global_init == 0) {
        ret = EVIEWITF_NOT_INITIALIZED;
    } else {
        /* Prepare TX buffer */
        request[0] = EVIEWITF_MFIS_FCT_DEINIT;

        /* Send request to R7 */
        ret = mfis_send_request(request);
        if ((ret < EVIEWITF_OK) || (request[0] != EVIEWITF_MFIS_FCT_DEINIT) ||
            (request[1] != EVIEWITF_MFIS_FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
        }
    }

    if (ret == EVIEWITF_OK) {
        eviewitf_global_init = 0;
    }

    mfis_deinit();

    /* End of critical section */
    pthread_mutex_unlock(&eviewitf_deinit_mutex);

    return ret;
}

/**
 * @fn eviewitf_ret_t camera_display(int cam_id)
 * @brief Request R7 to select camera device as display input
 *
 * @param cam_id camera identifier
 * @return Return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t camera_display(int cam_id) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Prepare TX buffer */
    request[0] = EVIEWITF_MFIS_FCT_SET_DISPLAY;
    request[1] = cam_id;
    /* Send request to R7 */
    ret = mfis_send_request(request);
    if (ret < EVIEWITF_OK) {
        ret = EVIEWITF_FAIL;
    } else {
        /* Check returned answer state */
        if ((request[0] != EVIEWITF_MFIS_FCT_SET_DISPLAY) && (request[1] != EVIEWITF_MFIS_FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * @fn eviewitf_display_select_camera(int cam_id)
 * @brief Select a camera input to be displayed on the screen connected to the eCube
 * @ingroup display
 *
 * @param[in] cam_id: id of the camera
 * @return return code as specified by the eviewitf_ret_t enumeration.
 *
 * Replace the currently displayed camera or streamer.
 */
eviewitf_ret_t eviewitf_display_select_camera(int cam_id) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    device_object_t *device = get_device_object(cam_id + EVIEWITF_OFFSET_CAMERA);

    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (ret == EVIEWITF_OK) {
        if (device->operations.display == NULL) {
            ret = EVIEWITF_FAIL;
        } else {
            ret = device->operations.display(cam_id + EVIEWITF_OFFSET_CAMERA);
        }
    }

    return ret;
}

/**
 * @fn eviewitf_display_select_streamer(int streamer_id)
 * @brief Select a streamer to be printed on the screen connected to the eCube
 * @ingroup display
 *
 * @param[in] streamer_id: id of the streamer
 * @return return code as specified by the eviewitf_ret_t enumeration.
 *
 * Replace the currently displayed camera or streamer.
 */
eviewitf_ret_t eviewitf_display_select_streamer(int streamer_id) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    device_object_t *device = get_device_object(streamer_id + EVIEWITF_OFFSET_STREAMER);

    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (ret == EVIEWITF_OK) {
        if (device->operations.display == NULL) {
            ret = EVIEWITF_FAIL;
        } else {
            ret = device->operations.display(streamer_id + EVIEWITF_OFFSET_STREAMER);
        }
    }

    return ret;
}

/**
 * @fn eviewitf_display_select_blender(int blender_id)
 * @brief Select a blender to be displayed, over the currently selected camera or streamer, on the screen connected to
 * the eCube.
 * @ingroup display
 *
 * @param[in] blender_id: id of the blender
 * @return return code as specified by the eviewitf_ret_t enumeration.
 *
 * Calling this function with blender_id not included between 0 and EVIEWITF_MAX_BLENDER – 1 (API macros) deactivates
 * the blender (no more overlay on the currently displayed camera or streamer).
 */
eviewitf_ret_t eviewitf_display_select_blender(int blender_id) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Test blender id */
    if ((blender_id < -1) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        request[0] = EVIEWITF_MFIS_FCT_SET_BLENDING;
        if (blender_id < 0) {
            request[1] = 0;
        } else {
            request[1] = 1;
            request[2] = blender_id;
        }
        ret = mfis_send_request(request);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (request[0] != EVIEWITF_MFIS_FCT_SET_BLENDING) {
                ret = EVIEWITF_FAIL;
            }
            if (request[1] == EVIEWITF_MFIS_FCT_RETURN_ERROR) {
                ret = EVIEWITF_FAIL;
            }
            if (request[1] == EVIEWITF_MFIS_FCT_INV_PARAM) {
                ret = EVIEWITF_INVALID_PARAM;
            }
        }
    }

    return ret;
}

eviewitf_ret_t eviewitf_set_R7_heartbeat_mode(uint32_t mode) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Prepare TX buffer */
    request[0] = EVIEWITF_MFIS_FCT_SET_HEARTBEAT;
    request[1] = mode;

    /* Send request to R7 */
    ret = mfis_send_request(request);
    if ((ret < EVIEWITF_OK) || (request[0] != EVIEWITF_MFIS_FCT_SET_HEARTBEAT) ||
        (request[1] != EVIEWITF_MFIS_FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    return ret;
}

eviewitf_ret_t eviewitf_set_R7_boot_mode(uint32_t mode) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Prepare TX buffer */
    request[0] = EVIEWITF_MFIS_FCT_SET_BOOT_MODE;
    request[1] = mode;

    /* Send request to R7 */
    ret = mfis_send_request(request);
    if ((ret < EVIEWITF_OK) || (request[0] != EVIEWITF_MFIS_FCT_SET_BOOT_MODE) ||
        (request[1] != EVIEWITF_MFIS_FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    return ret;
}

/**
 * @fn eviewitf_get_eviewitf_version
 * @brief Get the version of eViewItf.
 * @ingroup version
 *
 * @return returns a pointer on a string containing the eViewItf version number.
 */
const char *eviewitf_get_eviewitf_version(void) { return VERSION; }

/**
 * @fn eviewitf_get_eview_version
 * @brief Retrieve eView version
 * @ingroup version
 *
 * @return returns a pointer on a string containing the eView version number.
 *
 * Retrieve the running eView version.
 */
const char *eviewitf_get_eview_version(void) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int i = 0;
    int j = 0;
    int size_div = 0;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    if (strlen(eview_version) != 0) {
        return eview_version;
    }

    /* Prepare TX buffer */
    request[0] = EVIEWITF_MFIS_FCT_GET_EVIEW_VERSION;

    /* Send request to R7 */
    ret = mfis_send_request(request);
    if ((ret < EVIEWITF_OK) || (request[0] != EVIEWITF_MFIS_FCT_GET_EVIEW_VERSION) ||
        (request[1] != EVIEWITF_MFIS_FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
        return NULL;
    } else {
        if (request[2] == 20) /* 5 uint32_t split in 4 uint_8t is the maximum available*/
        {
            size_div = 5;
        } else if ((request[2] - ((request[2] / 4) * 4)) >= 1) { /* check if there is a rest */
            size_div = (request[2] / 4) + 1;                     /* +1 to get the rest of the division */
        } else {
            size_div = (request[2] / 4); /* no rest */
        }
        for (i = 0; i < size_div; i++) {
            for (j = 0; j < 4; j++) {
                eview_version[(i * 4) + j] = (char)(request[3 + i] >> (24 - (j * 8)));
            }
        }
        return eview_version;
    }
}

eviewitf_ret_t eviewitf_get_monitoring_info(uint32_t *data, uint8_t size) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    uint8_t i;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Prepare TX buffer */
    request[0] = EVIEWITF_MFIS_FCT_GET_MONITORING_INFO;

    /* Send request to R7 */
    ret = mfis_send_request(request);
    if ((ret < EVIEWITF_OK) || (request[0] != EVIEWITF_MFIS_FCT_GET_MONITORING_INFO) ||
        (request[1] != EVIEWITF_MFIS_FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    if (size > EVIEWITF_MONITORING_INFO_SIZE) size = EVIEWITF_MONITORING_INFO_SIZE;
    /* Max 6 bytes of usefull data */
    for (i = 0; i < size; i++) {
        data[i] = request[i + 2];
    }

    return ret;
}

eviewitf_ret_t eviewitf_get_R7_boot_mode(uint32_t *mode) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Prepare TX buffer */
    request[0] = EVIEWITF_MFIS_FCT_GET_BOOT_MODE;

    /* Send request to R7 */
    ret = mfis_send_request(request);
    if ((ret < EVIEWITF_OK) || (request[0] != EVIEWITF_MFIS_FCT_GET_BOOT_MODE) ||
        (request[1] != EVIEWITF_MFIS_FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    } else {
        *mode = request[2];
    }

    return ret;
}
/**
 * @fn eviewitf_display_select_cropping
 * @brief Start the cropping with coordinates to R7
 *
 * @param[in] x1: set first coordinate X position
 * @param[in] y1: set first coordinate Y position
 * @param[in] x2: set second coordinate X position
 * @param[in] y2: set second coordinate Y position
 * @return Return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_display_select_cropping(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int32_t request[EVIEWITF_MFIS_MSG_SIZE] = {0};

    /* Prepare TX buffer */
    request[0] = EVIEWITF_MFIS_FCT_SET_CROPPING;
    request[1] = x1;
    request[2] = y1;
    request[3] = x2;
    request[4] = y2;

    /* Send request to R7 */
    ret = mfis_send_request(request);
    if ((ret < EVIEWITF_OK) || (request[0] != EVIEWITF_MFIS_FCT_SET_CROPPING) ||
        (request[1] != EVIEWITF_MFIS_FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    return ret;
}

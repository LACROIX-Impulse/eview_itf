/**
 * \file eviewitf.c
 * \brief Communication API between A53 and R7 CPUs
 * \author esoftthings
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mfis_communication.h"
#include "eviewitf_priv.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/
#define MAX_VERSION_SIZE 21

/******************************************************************************************
 * Private structures
 ******************************************************************************************/

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/

static uint8_t eviewitf_global_init = 0;
static char eview_version[MAX_VERSION_SIZE];

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/**
 * \fn eviewitf_is_initialized
 * \brief Check if initialization has been performed
 *
 * \return 0 if not initialized
 */
int eviewitf_is_initialized() { return eviewitf_global_init; }

/**
 * \fn eviewitf_init
 * \brief Initialize the eViewItf API
 * \ingroup eview
 *
 * \return Return code as specified by the eviewitf_return_code enumeration.
 *
 * Initialize the eViewItf API by opening a communication with eView and by retrieving devices information from eView.
 * This function must be called before any other function of this API.
 * Otherwise, the other functions will return the error code EVIEWITF_NOT_INITIALIZED (eviewitf_return_state).
 */
int eviewitf_init(void) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    mfis_init();

    /* Check if init has been done */
    if (eviewitf_global_init != 0) {
        ret = EVIEWITF_FAIL;
    } else {
        /* Prepare TX buffer */
        tx_buffer[0] = EVIEWITF_MFIS_FCT_INIT;

        /* Send request to R7 and check returned answer state*/
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < EVIEWITF_OK) || (rx_buffer[0] != EVIEWITF_MFIS_FCT_INIT) || (rx_buffer[1] != FCT_RETURN_OK)) {
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

    return ret;
}

/**
 * \fn eviewitf_deinit
 * \brief De-initialize the eViewItf API
 * \ingroup eview
 *
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * De-initialize the eViewItf API by closing the communication with eView.
 */
int eviewitf_deinit(void) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Check if init has been done */
    if (eviewitf_global_init == 0) {
        ret = EVIEWITF_NOT_INITIALIZED;
    } else {
        /* Prepare TX buffer */
        tx_buffer[0] = EVIEWITF_MFIS_FCT_DEINIT;

        /* Send request to R7 */
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < EVIEWITF_OK) || (rx_buffer[0] != EVIEWITF_MFIS_FCT_DEINIT) || (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
        }
    }

    if (ret == EVIEWITF_OK) {
        eviewitf_global_init = 0;
    }

    mfis_deinit();

    return ret;
}

/**
 * \fn camera_display
 * \brief Request R7 to select camera device as display input
 *
 * \return state of the function. Return 0 if okay
 */
int camera_display(int cam_id) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = EVIEWITF_MFIS_FCT_SET_DISPLAY;
    tx_buffer[1] = cam_id;
    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if (ret < EVIEWITF_OK) {
        ret = EVIEWITF_FAIL;
    } else {
        /* Check returned answer state */
        if ((rx_buffer[0] != EVIEWITF_MFIS_FCT_SET_DISPLAY) && (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_display_select_camera(int cam_id)
 * \brief Select a camera input to be displayed on the screen connected to the eCube
 * \ingroup display
 *
 * \param[in] cam_id: id of the camera
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * Replace the currently displayed camera or streamer.
 */
int eviewitf_display_select_camera(int cam_id) {
    int ret = EVIEWITF_OK;
    device_object *device = get_device_object(cam_id + EVIEWITF_OFFSET_CAMERA);

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
 * \fn eviewitf_display_select_streamer(int streamer_id)
 * \brief Select a streamer to be printed on the screen connected to the eCube
 * \ingroup display
 *
 * \param[in] streamer_id: id of the streamer
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * Replace the currently displayed camera or streamer.
 */
int eviewitf_display_select_streamer(int streamer_id) {
    int ret = EVIEWITF_OK;
    device_object *device = get_device_object(streamer_id + EVIEWITF_OFFSET_STREAMER);

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
 * \fn eviewitf_camera_get_parameter(int cam_id, uint32_t reg_address, uint32_t* reg_value)
 * \brief Get a parameter of a camera.
 * \ingroup cameras
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] reg_address Register address
 * \param[out] reg_value Register Value
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_parameter(int cam_id, uint32_t reg_address, uint32_t *reg_value) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else if (reg_value == NULL) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        memset(tx_buffer, 0, sizeof(tx_buffer));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        /* Prepare TX buffer */
        tx_buffer[0] = EVIEWITF_MFIS_FCT_CAM_GET_REGISTER;
        tx_buffer[1] = cam_id;
        tx_buffer[2] = 0;
        tx_buffer[3] = reg_address;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != EVIEWITF_MFIS_FCT_CAM_GET_REGISTER) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_ERROR) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_INV_PARAM) {
                ret = EVIEWITF_INVALID_PARAM;
            }
            if (rx_buffer[1] == FCT_RETURN_BLOCKED) {
                ret = EVIEWITF_BLOCKED;
            }
        }
        *reg_value = (uint32_t)rx_buffer[2];
    }
    return ret;
}

/**
 * \fn eviewitf_camera_set_parameter(int cam_id, uint32_t reg_address, uint32_t reg_value)
 * \brief Set a parameter of a camera.
 * \ingroup cameras
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] reg_address Register address
 * \param[in] reg_value Register Value to set
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_parameter(int cam_id, uint32_t reg_address, uint32_t reg_value) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        memset(tx_buffer, 0, sizeof(tx_buffer));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        /* Prepare TX buffer */
        tx_buffer[0] = EVIEWITF_MFIS_FCT_CAM_SET_REGISTER;
        tx_buffer[1] = cam_id;
        tx_buffer[2] = 0;
        tx_buffer[3] = reg_address;
        tx_buffer[4] = (int32_t)reg_value;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != EVIEWITF_MFIS_FCT_CAM_SET_REGISTER) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_ERROR) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_INV_PARAM) {
                ret = EVIEWITF_INVALID_PARAM;
            }
            if (rx_buffer[1] == FCT_RETURN_BLOCKED) {
                ret = EVIEWITF_BLOCKED;
            }
        }
    }
    return ret;
}

/**
 * \fn eviewitf_display_select_blender(int blender_id)
 * \brief Select a blender to be displayed, over the currently selected camera or streamer, on the screen connected to
 * the eCube.
 * \ingroup display
 *
 * \param[in] blender_id: id of the blender
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * Calling this function with blender_id not included between 0 and EVIEWITF_MAX_BLENDER – 1 (API macros) deactivates
 * the blender (no more overlay on the currently displayed camera or streamer).
 */
int eviewitf_display_select_blender(int blender_id) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    /* Test blender id */
    if ((blender_id < -1) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        memset(tx_buffer, 0, sizeof(tx_buffer));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        tx_buffer[0] = EVIEWITF_MFIS_FCT_SET_BLENDING;
        if (blender_id < 0) {
            tx_buffer[1] = 0;
        } else {
            tx_buffer[1] = 1;
            tx_buffer[2] = blender_id;
        }
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != EVIEWITF_MFIS_FCT_SET_BLENDING) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_ERROR) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_INV_PARAM) {
                ret = EVIEWITF_INVALID_PARAM;
            }
        }
    }

    return ret;
}

int eviewitf_set_camera_fps(int cam_id, uint32_t fps) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        memset(tx_buffer, 0, sizeof(tx_buffer));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        tx_buffer[0] = EVIEWITF_MFIS_FCT_CAM_SET_FPS;
        tx_buffer[1] = cam_id;
        tx_buffer[2] = (int32_t)fps;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != EVIEWITF_MFIS_FCT_CAM_SET_FPS) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_ERROR) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_BLOCKED) {
                ret = EVIEWITF_BLOCKED;
            }
            if (rx_buffer[1] == FCT_INV_PARAM) {
                ret = EVIEWITF_INVALID_PARAM;
            }
        }
    }
    return ret;
}

/**
 * \fn eviewitf_set_R7_heartbeat_mode(uint32_t mode)
 * \brief Activate or deactivate eView heartbeat.
 * \ingroup eview
 *
 * \param[in] mode 0 to deactivate heartbeat other to activate it
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The eView heartbeat can be activated to check if eView is still running as it should.
 * With the heartbeat activated, eView will regularly send a message over the eCube’s USB Debug port.
 * This is a debugging function. This function should not be used in a normal behavior.
 * However, it can help to identify the cause of an EVIEWITF_BLOCKED (eviewitf_return_code) error code.
 */
int eviewitf_set_R7_heartbeat_mode(uint32_t mode) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = EVIEWITF_MFIS_FCT_SET_HEARTBEAT;
    tx_buffer[1] = mode;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != EVIEWITF_MFIS_FCT_SET_HEARTBEAT) || (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    return ret;
}

/**
 * \fn eviewitf_set_R7_boot_mode(uint32_t mode)
 * \brief Set a specific boot mode to eView.
 * \ingroup eview
 *
 * \param[in] mode requets a specific R7 boot mode
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * An eView specific mode can be set under peculiar conditions.
 * This function is not needed most of the time. It can be used to tune the eView’s behavior for some customers’
 * requests.
 */
int eviewitf_set_R7_boot_mode(uint32_t mode) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = EVIEWITF_MFIS_FCT_SET_BOOT_MODE;
    tx_buffer[1] = mode;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != EVIEWITF_MFIS_FCT_SET_BOOT_MODE) || (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    return ret;
}

/**
 * \fn eviewitf_get_eviewitf_version
 * \brief Get the version of eViewItf.
 * \ingroup version
 *
 * \return returns a pointer on a string containing the eViewItf version number.
 */
const char *eviewitf_get_eviewitf_version(void) { return VERSION; }

/**
 * \fn eviewitf_get_eview_version
 * \brief Retrieve eView version
 * \ingroup version
 *
 * \return returns a pointer on a string containing the eView version number.
 *
 * Retrieve the running eView version.
 */
const char *eviewitf_get_eview_version(void) {
    int ret = EVIEWITF_OK;
    int i = 0;
    int j = 0;
    int size_div = 0;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    if (strlen(eview_version) != 0) {
        return eview_version;
    }
    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = EVIEWITF_MFIS_FCT_GET_EVIEW_VERSION;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != EVIEWITF_MFIS_FCT_GET_EVIEW_VERSION) ||
        (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
        return NULL;
    } else {
        if (rx_buffer[2] == 20) /* 5 uint32_t split in 4 uint_8t is the maximum available*/
        {
            size_div = 5;
        } else if ((rx_buffer[2] - ((rx_buffer[2] / 4) * 4)) >= 1) { /* check if there is a rest */
            size_div = (rx_buffer[2] / 4) + 1;                       /* +1 to get the rest of the division */
        } else {
            size_div = (rx_buffer[2] / 4); /* no rest */
        }
        for (i = 0; i < size_div; i++) {
            for (j = 0; j < 4; j++) {
                eview_version[(i * 4) + j] = (char)(rx_buffer[3 + i] >> (24 - (j * 8)));
            }
        }
        return eview_version;
    }
}

/**
 * \fn eviewitf_get_monitoring_info(uint32_t* data, uint8_t size)
 * \brief Request R7 to get monitoring info.
 * \ingroup eview
 *
 * \param[out] data pointer where to store monitoring info
 * \param[in] size size of the data table, should not be greater than EVIEWITF_MONITORING_INFO_SIZE
 * \return state of the function. Return 0 if okay
 *
 * Content is voluntary not explicitly described in this interface, can be project specific.
 */
int eviewitf_get_monitoring_info(uint32_t *data, uint8_t size) {
    int ret = EVIEWITF_OK;
    uint8_t i;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = EVIEWITF_MFIS_FCT_GET_MONITORING_INFO;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != EVIEWITF_MFIS_FCT_GET_MONITORING_INFO) ||
        (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    if (size > EVIEWITF_MONITORING_INFO_SIZE) size = EVIEWITF_MONITORING_INFO_SIZE;
    /* Max 6 bytes of usefull data */
    for (i = 0; i < size; i++) {
        data[i] = rx_buffer[i + 2];
    }

    return ret;
}

/**
 * \fn eviewitf_display_select_cropping
 * \brief Start the cropping with coordinates to R7
 *
 * \param[in] x1: set first coordinate X position
 * \param[in] y1: set first coordinate Y position
 * \param[in] x2: set second coordinate X position
 * \param[in] y2: set second coordinate Y position
 * \return state of the function. Return 0 if okay
 */
int eviewitf_display_select_cropping(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = EVIEWITF_MFIS_FCT_SET_CROPPING;
    tx_buffer[1] = x1;
    tx_buffer[2] = y1;
    tx_buffer[3] = x2;
    tx_buffer[4] = y2;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != EVIEWITF_MFIS_FCT_SET_CROPPING) || (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    return ret;
}

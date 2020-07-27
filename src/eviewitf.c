/**
 * \file eviewitf.c
 * \brief Communication API between A53 and R7 CPUs
 * \author esoftthings
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <poll.h>
#include <dlfcn.h>
#include "mfis_communication.h"
#include "eviewitf.h"
#include "eviewitf_priv.h"
#include "eviewitf_ssd.h"

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

/* Blending attributes */
static struct eviewitf_mfis_blending_attributes all_blendings_attributes[EVIEWITF_MAX_BLENDER] = {0};

/* Camera objects */
static struct eviewitf_camera_object all_cameras_objects[EVIEWITF_MAX_CAMERA + EVIEWITF_MAX_STREAMER] = {0};

static uint8_t eviewitf_global_init = 0;
static char eview_version[MAX_VERSION_SIZE];

/******************************************************************************************
 * Functions
 ******************************************************************************************/
/* Private function used to retrieved cam buffer during api initialization.
 Doesn't need to be exposed in API */
/**
 * \fn eviewitf_mfis_get_cam_attributes
 * \brief Get the cameras attributes
 *
 * \param [inout] cameras_attributes: Pointer to a table of struct eviewitf_mfis_camera_attributes
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
static int eviewitf_mfis_get_cam_attributes(struct eviewitf_mfis_camera_attributes *cameras_attributes) {
    int ret = EVIEWITF_OK;

    /* Check input parameter */
    if (cameras_attributes == NULL) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    /* Get the cameras attributes */
    if (ret == EVIEWITF_OK) {
        ret = mfis_get_cam_attributes(cameras_attributes);
        if (ret != EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_mfis_get_blend_attributes
 * \brief Get the blendings attributes
 *
 * \param [inout] blendings_attributes: Pointer to a table of struct eviewitf_mfis_blending_attributes
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
static int eviewitf_mfis_get_blend_attributes(struct eviewitf_mfis_blending_attributes *blendings_attributes) {
    int ret = EVIEWITF_OK;

    /* Check input parameter */
    if (blendings_attributes == NULL) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    /* Get the blendings attributes */
    if (ret == EVIEWITF_OK) {
        ret = mfis_get_blend_attributes(blendings_attributes);
        if (ret != EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_is_initialized
 * \brief Check if initialization has been performed
 *
 * \return 0 if not initialized
 */
int eviewitf_is_initialized() { return eviewitf_global_init; }

/**
 * \fn eviewitf_get_camera_object
 * \brief Get a pointer on the camera object
 *
 * \param [in] cam_id: Camera id
 *
 * \return pointer on camera object structure
 */
struct eviewitf_camera_object *eviewitf_get_camera_object(int cam_id) {
    if (cam_id < 0 || cam_id >= EVIEWITF_MAX_CAMERA + EVIEWITF_MAX_STREAMER) {
        return NULL;
    }
    return &all_cameras_objects[cam_id];
}

/**
 * \fn eviewitf_get_blender_attributes
 * \brief Get a pointer on the blender attributes
 *
 * \param [in] blender_id: Blender id
 *
 * \return pointer on blender attributes structure
 */
struct eviewitf_mfis_blending_attributes *eviewitf_get_blender_attributes(int blender_id) {
    if (blender_id < 0 || blender_id >= EVIEWITF_MAX_BLENDER) {
        return NULL;
    }
    return &all_blendings_attributes[blender_id];
}

/**
 * \fn eviewitf_init
 * \brief Init MFIS driver on R7 side
 *
 * \param [in] camera id
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_init(void) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[EVIEWITF_MFIS_MSG_SIZE], rx_buffer[EVIEWITF_MFIS_MSG_SIZE];
    struct eviewitf_mfis_camera_attributes cameras_attributes[EVIEWITF_MAX_CAMERA + EVIEWITF_MAX_STREAMER] = {0};

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

        /* Get the cameras attributes */
        if (ret == EVIEWITF_OK) {
            ret = eviewitf_mfis_get_cam_attributes(cameras_attributes);
        }
        /* Fill camera objects structure */
        if (ret == EVIEWITF_OK) {
            /* Set camera operations */
            for (int i = 0; i < EVIEWITF_MAX_CAMERA + EVIEWITF_MAX_STREAMER; i++) {
                /* Copy attributes */
                memcpy(&all_cameras_objects[i].camera_attributes, &cameras_attributes[i], sizeof(struct eviewitf_mfis_camera_attributes));
                /* Set operations */
                switch (cameras_attributes[i].cam_type)
                {
                    case EVIEWITF_MFIS_CAM_TYPE_GENERIC:
                        all_cameras_objects[i].camera_operations.open = camera_generic_open;
                        all_cameras_objects[i].camera_operations.close = camera_generic_close;
                        all_cameras_objects[i].camera_operations.write = NULL;
                        all_cameras_objects[i].camera_operations.read = camera_generic_read;
                        all_cameras_objects[i].camera_operations.display = generic_camera_display;
                        break;
                    case EVIEWITF_MFIS_CAM_TYPE_VIRTUAL:
                        all_cameras_objects[i].camera_operations.open = camera_streamer_open;
                        all_cameras_objects[i].camera_operations.close = camera_streamer_close;
                        all_cameras_objects[i].camera_operations.write = camera_streamer_write;
                        all_cameras_objects[i].camera_operations.read = NULL;
                        all_cameras_objects[i].camera_operations.display = generic_camera_display;
                        break;
                    case EVIEWITF_MFIS_CAM_TYPE_SEEK:
                        all_cameras_objects[i].camera_operations.open = camera_generic_open;
                        all_cameras_objects[i].camera_operations.close = camera_generic_close;
                        all_cameras_objects[i].camera_operations.write = NULL;
                        all_cameras_objects[i].camera_operations.read = camera_generic_read;
                        all_cameras_objects[i].camera_operations.display = generic_camera_display;
                        break;
                    
                    default:
                        all_cameras_objects[i].camera_operations.open = NULL;
                        all_cameras_objects[i].camera_operations.close = NULL;
                        all_cameras_objects[i].camera_operations.write = NULL;
                        all_cameras_objects[i].camera_operations.read = NULL;
                        all_cameras_objects[i].camera_operations.display = NULL;

                        break;
                }
            }
        }


        /* Get the blendings attributes */
        if (ret == EVIEWITF_OK) {
            ret = eviewitf_mfis_get_blend_attributes(all_blendings_attributes);
        }

        if (ret == EVIEWITF_OK) {
            eviewitf_global_init = 1;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_deinit
 * \brief Deinit MFIS driver on R7 side
 *
 * \return state of the function. Return 0 if okay
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
 * \fn generic_camera_display
 * \brief Request R7 to select camera device as display input
 *
 * \return state of the function. Return 0 if okay
 */
int generic_camera_display(int cam_id) {
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
 * \fn eviewitf_display_select_camera
 * \brief Request R7 to select camera as display input
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_display_select_camera(int cam_id) {
    int ret = EVIEWITF_OK;

    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (ret == EVIEWITF_OK) {
        if (all_cameras_objects[cam_id].camera_operations.display == NULL) {
            ret = EVIEWITF_FAIL;
        }
        else  {
            ret = generic_camera_display(cam_id);
        }
    }

    return ret;
}

/**
 * \fn eviewitf_display_select_streamer
 * \brief Request R7 to select streamer as display input
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_display_select_streamer(int streamer_id) {
    int ret = EVIEWITF_OK;

    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (ret == EVIEWITF_OK) {
        if (all_cameras_objects[streamer_id + EVIEWITF_MAX_CAMERA].camera_operations.display == NULL) {
            ret = EVIEWITF_FAIL;
        }
        else  {
            ret = generic_camera_display(streamer_id + EVIEWITF_MAX_CAMERA);
        }
    }

    return ret;
}

/**
 * \fn eviewitf_camera_get_parameter
 * \brief Request R7 to get a register value
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param reg_adress: Register address
 * \param *reg_value: Register Value
 * \return state of the function. Return 0 if okay
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
 * \fn eviewitf_camera_set_parameter
 * \brief Request R7 to set a register to a value
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param reg_adress: Register address
 * \param reg_value: Register Value to set
 * \return state of the function. Return 0 if okay
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
 * \fn eviewitf_display_select_blender
 * \brief Start / stop the blending (use -1) to stop
 *
 * \return state of the function. Return 0 if okay
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
 * \fn eviewitf_set_R7_heartbeat_mode
 * \brief Activate/deactivate R7 heartbeat
 *
 * \param in mode: 0 to deactivate heartbeat other to activate it
 * \return state of the function. Return 0 if okay
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
 * \fn eviewitf_set_R7_boot_mode
 * \brief Set the R7 boot mode
 *
 * \param in mode: requets a specific R7 boot mode
 * \return state of the function. Return 0 if okay
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
 * \brief Return the eViewitf lib version
 *
 * \return state of the function. Return version if okay, NULL if fail
 */
const char *eviewitf_get_eviewitf_version(void) { return VERSION; }

/**
 * \fn eviewitf_get_eview_version
 * \brief Retrieve eview version
 *
 * \param in version: return version number
 * \return state of the function. Return 0 if okay
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
 * \fn eviewitf_display_select_cropping
 * \brief Start the cropping with coordinates to R7
 *
 * \param in x1: set first coordinate X position
 * \param in y1: set first coordinate Y position
 * \param in x2: set second coordinate X position
 * \param in y2: set second coordinate Y position
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

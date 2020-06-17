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
typedef struct {
    void *handle_plugin;
    char *(*get_lib_version)();
    char *(*get_seek_lib_version)();
    int (*init_all_cameras)(int nb_cam);
    int (*deinit_all_cameras)(int nb_cam);
    int (*start_camera)(int cam_id);
    int (*stop_camera)(int cam_id);
    int (*get_camera_setting)(int cam_id, int setting_nb, int *setting_value);
    int (*set_camera_setting)(int cam_id, int setting_nb, int setting_value);
    int (*get_camera_frame)(int cam_id, float **temperature, uint32_t **display);
} eviewitf_seek_plugin_handle;
char *seek_version = NULL;
char *seek_plugin_version = NULL;
/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/
typedef enum {
    FCT_INIT = 0,
    FCT_DEINIT = 1,
    FCT_CAM_GET_REGISTER = 10,
    FCT_CAM_SET_REGISTER = 11,
    FCT_CAM_SET_FPS = 12,
    FCT_CAM_RESET = 13,
    FCT_SET_HEARTBEAT = 20,
    FCT_SET_BOOT_MODE = 21,
    FCT_SET_DISPLAY = 22,
    FCT_SET_BLENDING = 23,
    FCT_SET_CROPPING = 24,
    FCT_GET_EVIEW_VERSION = 30,
    FCT_GET_MONITORING_INFO = 31,
    FCT_GET_CAM_BUFFERS = 32,
} fct_id_t;

/* Cameras attributes */
mfis_camera_attributes all_cameras_attributes[EVIEWITF_MAX_CAMERA] = {
    {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0},
    {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0},
    {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0},
    {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0}, {CAM_TYPE_NONE, 0, 0, 0, 0},
};

/* Blending attributes */
mfis_blending_attributes all_blendings_attributes[EVIEWITF_MAX_BLENDING] = {
    {0, 0, 0, 0},
    {0, 0, 0, 0},
};

uint8_t global_init = 0;
char eview_version[MAX_VERSION_SIZE];
eviewitf_seek_plugin_handle seek_plugin_handle;
/******************************************************************************************
 * Functions
 ******************************************************************************************/

/* Private function used to retrieved cam buffer during api initialization.
 Doesn't need to be exposed in API */
/**
 * \fn eviewitf_mfis_get_cam_attributes
 * \brief Get the cameras attributes
 *
 * \param [inout] cameras_attributes: Pointer to a table of mfis_camera_attributes
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
static int eviewitf_mfis_get_cam_attributes(mfis_camera_attributes *cameras_attributes) {
    int ret = EVIEWITF_OK;

    /* Check input parameter */
    if (cameras_attributes == NULL) {
        printf("Error eviewitf_mfis_get_cam_attributes called with a null parameter\n");
        ret = EVIEWITF_INVALID_PARAM;
    }

    /* Get the cameras attributes */
    if (ret == EVIEWITF_OK) {
        ret = mfis_get_cam_attributes(cameras_attributes);
        if (ret != EVIEWITF_OK) {
            printf("Error while retrieving cameras attributes\n");
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_mfis_get_blend_attributes
 * \brief Get the blendings attributes
 *
 * \param [inout] blendings_attributes: Pointer to a table of mfis_blending_attributes
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
static int eviewitf_mfis_get_blend_attributes(mfis_blending_attributes *blendings_attributes) {
    int ret = EVIEWITF_OK;

    /* Check input parameter */
    if (blendings_attributes == NULL) {
        printf("Error eviewitf_mfis_get_blend_attributes called with a null parameter\n");
        ret = EVIEWITF_INVALID_PARAM;
    }

    /* Get the blendings attributes */
    if (ret == EVIEWITF_OK) {
        ret = mfis_get_blend_attributes(blendings_attributes);
        if (ret != EVIEWITF_OK) {
            printf("Error while retrieving blendings attributes\n");
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_init_api
 * \brief Deinit MFIS driver on R7 side
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_init_api(void) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    mfis_init();

    /* Check if init has been done */
    if (global_init != 0) {
        printf("eviewitf_init_api already done\n");
        ret = EVIEWITF_FAIL;
    } else {
        /* Prepare TX buffer */
        tx_buffer[0] = FCT_INIT;

        /* Send request to R7 and check returned answer state*/
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_INIT) || (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
        }

        /* Get the cameras attributes */
        if (ret == EVIEWITF_OK) {
            ret = eviewitf_mfis_get_cam_attributes(all_cameras_attributes);
            if (ret != EVIEWITF_OK) {
                printf("eviewitf_init_api: Error in eviewitf_mfis_get_cam_attributes\n");
            }
        }

        /* Get the blendings attributes */
        if (ret == EVIEWITF_OK) {
            ret = eviewitf_mfis_get_blend_attributes(all_blendings_attributes);
            if (ret != EVIEWITF_OK) {
                printf("eviewitf_init_api: Error in eviewitf_mfis_get_blend_attributes\n");
            }
        }

        if (ret == EVIEWITF_OK) {
            global_init = 1;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_deinit_api
 * \brief Deinit MFIS driver on R7 side
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_deinit_api(void) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Check if init has been done */
    if (global_init == 0) {
        printf("eviewitf_init_api never done\n");
        ret = EVIEWITF_FAIL;
    } else {
        /* Prepare TX buffer */
        tx_buffer[0] = FCT_DEINIT;

        /* Send request to R7 */
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_DEINIT) || (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
        }
    }

    mfis_deinit();

    return ret;
}

/**
 * \fn eviewitf_set_display_cam
 * \brief Request R7 to change camera used on display
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_set_display_cam(int cam_id) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_SET_DISPLAY;
    tx_buffer[1] = cam_id;
    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if (ret < EVIEWITF_OK) {
        ret = EVIEWITF_FAIL;
    } else {
        /* Check returned answer state */
        if ((rx_buffer[0] != FCT_SET_DISPLAY) && (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_record_cam
 * \brief Request R7 to change camera used on display
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param delay: duration of the record in seconds
 * \return state of the function. Return 0 if okay
 */
int eviewitf_record_cam(int cam_id, int delay) {
    int ret = EVIEWITF_OK;
    char *record_dir = NULL;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_REAL_CAMERA)) {
        printf("Invalid camera id\n");
        printf("Please choose a real camera for the record\n");
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        ssd_get_output_directory(&record_dir);
        printf("SSD storage directory %s \n", record_dir);
        ret = ssd_save_camera_stream(cam_id, delay, record_dir, all_cameras_attributes[cam_id].buffer_size);
        free(record_dir);
    }
    return ret;
}

/**
 * \fn eviewitf_get_camera_param
 * \brief Request R7 to get a register value
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param cam_type: Camera type (ie: OV2311)
 * \param reg_adress: Register address
 * \param *reg_value: Register Value
 * \return state of the function. Return 0 if okay
 */
int eviewitf_get_camera_param(int cam_id, int cam_type, uint32_t reg_address, uint32_t *reg_value) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_REAL_CAMERA)) {
        printf("Invalid camera id\n");
        printf("Please choose a real camera for the get param\n");
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        memset(tx_buffer, 0, sizeof(tx_buffer));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        /* Prepare TX buffer */
        tx_buffer[0] = FCT_CAM_GET_REGISTER;
        tx_buffer[1] = cam_id;
        tx_buffer[2] = cam_type;
        tx_buffer[3] = reg_address;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != FCT_CAM_GET_REGISTER) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_ERROR) {
                ret = EVIEWITF_FAIL;
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
 * \fn eviewitf_set_camera_param
 * \brief Request R7 to set a register to a value
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param cam_type: Camera type (ie: OV2311)
 * \param reg_adress: Register address
 * \param reg_value: Register Value to set
 * \return state of the function. Return 0 if okay
 */
int eviewitf_set_camera_param(int cam_id, int cam_type, uint32_t reg_address, uint32_t reg_value) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_REAL_CAMERA)) {
        printf("Invalid camera id\n");
        printf("Please choose a real camera for the set param\n");
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        memset(tx_buffer, 0, sizeof(tx_buffer));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        /* Prepare TX buffer */
        tx_buffer[0] = FCT_CAM_SET_REGISTER;
        tx_buffer[1] = cam_id;
        tx_buffer[2] = cam_type;
        tx_buffer[3] = reg_address;
        tx_buffer[4] = (int32_t)reg_value;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != FCT_CAM_SET_REGISTER) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_ERROR) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_BLOCKED) {
                ret = EVIEWITF_BLOCKED;
            }
        }
    }
    return ret;
}

int eviewitf_reboot_cam(int cam_id) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_REAL_CAMERA)) {
        printf("Invalid camera id\n");
        printf("Please choose a real camera for the reboot\n");
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        memset(tx_buffer, 0, sizeof(tx_buffer));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        tx_buffer[0] = FCT_CAM_RESET;
        tx_buffer[1] = cam_id;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != FCT_CAM_RESET) {
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

/**
 * \fn eviewitf_stop_blending
 * \brief Stop the blending
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_start_blending(int blending_id) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    tx_buffer[0] = FCT_SET_BLENDING;
    tx_buffer[1] = 1;
    tx_buffer[2] = blending_id;
    ret = mfis_send_request(tx_buffer, rx_buffer);

    if (ret < EVIEWITF_OK) {
        ret = EVIEWITF_FAIL;
    } else {
        /* Check returned answer state */
        if (rx_buffer[0] != FCT_SET_BLENDING) {
            ret = EVIEWITF_FAIL;
        }
        if (rx_buffer[1] == FCT_RETURN_ERROR) {
            ret = EVIEWITF_FAIL;
        }
        if (rx_buffer[1] == FCT_INV_PARAM) {
            ret = EVIEWITF_INVALID_PARAM;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_stop_blending
 * \brief Stop the blending
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_stop_blending(void) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    tx_buffer[0] = FCT_SET_BLENDING;
    tx_buffer[1] = 0;
    ret = mfis_send_request(tx_buffer, rx_buffer);

    if (ret < EVIEWITF_OK) {
        ret = EVIEWITF_FAIL;
    } else {
        /* Check returned answer state */
        if (rx_buffer[0] != FCT_SET_BLENDING) {
            ret = EVIEWITF_FAIL;
        }
        if (rx_buffer[1] == FCT_RETURN_ERROR) {
            ret = EVIEWITF_FAIL;
        }
        if (rx_buffer[1] == FCT_INV_PARAM) {
            ret = EVIEWITF_INVALID_PARAM;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_play_on_virtual_cam
 * \brief Update the frames to be printed on a virtual camera

 * \param in cam_id: id of the camera
 * \param in fps: fps to apply on the recording
 * \param in frames_dir: path to the recording
 * \return state of the function. Return 0 if okay
 */
int eviewitf_play_on_virtual_cam(int cam_id, int fps, char *frames_dir) {
    int ret = EVIEWITF_OK;

    /* Test API has been initialized */
    if (global_init == 0) {
        printf("Please call eviewitf_init_api first\n");
        ret = EVIEWITF_FAIL;
    }

    if (EVIEWITF_OK == ret) {
        /* Test camera id */
        if ((cam_id < EVIEWITF_MAX_REAL_CAMERA) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
            printf("Invalid camera id\n");
            printf("Please choose a virtual camera for the playback\n");
            ret = EVIEWITF_INVALID_PARAM;
        }
    }

    if (EVIEWITF_OK == ret) {
        ret = ssd_set_virtual_camera_stream(cam_id, all_cameras_attributes[cam_id].buffer_size, fps, frames_dir);
        if (EVIEWITF_OK != ret) {
            printf("Error: Cannot play the stream on the virtual camera\n");
        }
    }

    return ret;
}

/**
 * \fn eviewitf_set_virtual_cam
 * \brief Set a virtual camera frame

 * \param in cam_id: id of the camera
 * \param in buffer_size: size of the virtual camera buffer
 * \param in buffer: virtual camera buffer
 * \return state of the function. Return 0 if okay
 */
int eviewitf_set_virtual_cam(int cam_id, uint32_t buffer_size, char *buffer) {
    int ret = EVIEWITF_OK;
    int cam_fd;
    int test_rw = 0;

    /* Test API has been initialized */
    if (global_init == 0) {
        printf("Please call eviewitf_init_api first\n");
        ret = EVIEWITF_FAIL;
    }

    /* Open the virtual camera to write in */
    cam_fd = open(mfis_device_filenames[cam_id], O_WRONLY);
    if (cam_fd == -1) {
        printf("Error opening camera file\n");
        ret = EVIEWITF_FAIL;
    }

    /* Write the frame in the virtual camera */
    test_rw = write(cam_fd, buffer, buffer_size);
    if ((-1) == test_rw) {
        printf("[Error] Write frame in the virtual camera\n");
        return EVIEWITF_FAIL;
    }

    /* Close the virtual camera file device */
    close(cam_fd);

    return ret;
}

/**
 * \fn eviewitf_set_blending_from_file
 * \brief Set a blending frame

 * \param in frame: path to the blending frame
 * \return state of the function. Return 0 if okay
 */
int eviewitf_set_blending_from_file(int blending_id, char *frame) {
    int ret = EVIEWITF_OK;

    /* Test API has been initialized */
    if (global_init == 0) {
        printf("Please call eviewitf_init_api first\n");
        ret = EVIEWITF_FAIL;
    }

    if ((blending_id < 0) || (blending_id > 1)) {
        printf("Error: eviewitf_set_blending_from_file, bad blending_id\n");
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (EVIEWITF_OK == ret) {
        ret = ssd_set_blending(blending_id, all_blendings_attributes[blending_id].buffer_size, frame);
        if (EVIEWITF_OK != ret) {
            printf("Error: Cannot set the blending\n");
        }
    }

    return ret;
}

/**
 * \fn eviewitf_set_blending
 * \brief Set a blending buffer

 * \param in buffer_size: size of the blending buffer
 * \param in buffer: blending buffer
 * \return state of the function. Return 0 if okay
 */
int eviewitf_write_blending(int blending_id, uint32_t buffer_size, char *buffer) {
    int ret = EVIEWITF_OK;
    int blend_fd;
    int test_rw = 0;

    /* Test API has been initialized */
    if (global_init == 0) {
        printf("Please call eviewitf_init_api first\n");
        ret = EVIEWITF_FAIL;
    }

    /* Open the blending device to write in */
    blend_fd = open(blending_interface[blending_id], O_WRONLY);
    if ((-1) == blend_fd) {
        printf("[Error] Opening the blendind device\n");
        return -1;
    }

    /* Write the frame in the blending device */
    test_rw = write(blend_fd, buffer, buffer_size);
    if ((-1) == test_rw) {
        printf("[Error] Write frame in the blending device\n");
        close(blend_fd);
        return -1;
    }

    close(blend_fd);

    return ret;
}

int eviewitf_set_camera_fps(int cam_id, uint32_t fps) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_REAL_CAMERA)) {
        printf("Invalid camera id\n");
        printf("Please choose a real camera for the fps\n");
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        memset(tx_buffer, 0, sizeof(tx_buffer));
        memset(rx_buffer, 0, sizeof(rx_buffer));

        tx_buffer[0] = FCT_CAM_SET_FPS;
        tx_buffer[1] = cam_id;
        tx_buffer[2] = (int32_t)fps;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != FCT_CAM_SET_FPS) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_ERROR) {
                ret = EVIEWITF_FAIL;
            }
            if (rx_buffer[1] == FCT_RETURN_BLOCKED) {
                ret = EVIEWITF_BLOCKED;
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
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_SET_HEARTBEAT;
    tx_buffer[1] = mode;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_SET_HEARTBEAT) || (rx_buffer[1] != FCT_RETURN_OK)) {
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
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_SET_BOOT_MODE;
    tx_buffer[1] = mode;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_SET_BOOT_MODE) || (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    return ret;
}

/**
 * \fn eviewitf_get_version
 * \brief Return the eViewitf lib version
 *
 * \return state of the function. Return version if okay, NULL if fail
 */
const char *eviewitf_get_lib_version(void) { return VERSION; }

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
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    if (strlen(eview_version) != 0) {
        return eview_version;
    }
    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_GET_EVIEW_VERSION;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_GET_EVIEW_VERSION) || (rx_buffer[1] != FCT_RETURN_OK)) {
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
 * \fn eviewitf_start_cropping
 * \brief Start the cropping with coordinates to R7
 *
 * \param in x1: set first coordinate X position
 * \param in y1: set first coordinate Y position
 * \param in x2: set second coordinate X position
 * \param in y2: set second coordinate Y position
 * \return state of the function. Return 0 if okay
 */
int eviewitf_start_cropping(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) {
    int ret = EVIEWITF_OK;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_SET_CROPPING;
    tx_buffer[1] = x1;
    tx_buffer[2] = y1;
    tx_buffer[3] = x2;
    tx_buffer[4] = y2;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_SET_CROPPING) || (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = EVIEWITF_FAIL;
    }

    return ret;
}

/**
 * \fn eviewitf_stop_cropping
 * \brief Stop the cropping on R7
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_stop_cropping(void) { return eviewitf_start_cropping(0, 0, 0, 0); }

/**
 * \fn eviewitf_import_seek_plugin
 * \brief Import fct definition of the seek plugin
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_import_seek_plugin(void) {
    if (seek_plugin_handle.handle_plugin == NULL) {
        seek_plugin_handle.handle_plugin = dlopen("libeview_itf_seek.so", RTLD_LAZY);
        if (seek_plugin_handle.handle_plugin == NULL) {
            printf("[Error] Issue while loading libeview_itf_seek.so, aborting \n");
            return EVIEWITF_FAIL;
        }
    }
    seek_plugin_handle.get_seek_lib_version =
        dlsym(seek_plugin_handle.handle_plugin, "eviewitf_seek_get_seek_lib_version");
    if (seek_plugin_handle.get_seek_lib_version == NULL) {
        printf("[Error] Issue while loading get_seek_lib_version in libeview_itf_seek.so, aborting \n");
        return EVIEWITF_FAIL;
    }
    seek_plugin_handle.get_lib_version = dlsym(seek_plugin_handle.handle_plugin, "eviewitf_seek_get_lib_version");
    if (seek_plugin_handle.get_lib_version == NULL) {
        printf("[Error] Issue while loading get_lib_version in libeview_itf_seek.so, aborting \n");
        return EVIEWITF_FAIL;
    }
    seek_plugin_handle.init_all_cameras = dlsym(seek_plugin_handle.handle_plugin, "eviewitf_seek_init_all_cameras");
    if (seek_plugin_handle.init_all_cameras == NULL) {
        printf("[Error] Issue while loading init_all_cameras in libeview_itf_seek.so, aborting \n");
        return EVIEWITF_FAIL;
    }
    seek_plugin_handle.deinit_all_cameras = dlsym(seek_plugin_handle.handle_plugin, "eviewitf_seek_deinit_all_cameras");
    if (seek_plugin_handle.deinit_all_cameras == NULL) {
        printf("[Error] Issue while loading deinit_all_cameras in libeview_itf_seek.so, aborting \n");
        return EVIEWITF_FAIL;
    }
    seek_plugin_handle.start_camera = dlsym(seek_plugin_handle.handle_plugin, "eviewitf_seek_start_camera");
    if (seek_plugin_handle.start_camera == NULL) {
        printf("[Error] Issue while loading start_camera in libeview_itf_seek.so, aborting \n");
        return EVIEWITF_FAIL;
    }
    seek_plugin_handle.stop_camera = dlsym(seek_plugin_handle.handle_plugin, "eviewitf_seek_stop_camera");
    if (seek_plugin_handle.stop_camera == NULL) {
        printf("[Error] Issue while loading stop_camera in libeview_itf_seek.so, aborting \n");
        return EVIEWITF_FAIL;
    }
    seek_plugin_handle.get_camera_setting = dlsym(seek_plugin_handle.handle_plugin, "eviewitf_seek_get_camera_setting");
    if (seek_plugin_handle.get_camera_setting == NULL) {
        printf("[Error] Issue while loading get_camera_setting in libeview_itf_seek.so, aborting \n");
        return EVIEWITF_FAIL;
    }
    seek_plugin_handle.set_camera_setting = dlsym(seek_plugin_handle.handle_plugin, "eviewitf_seek_set_camera_setting");
    if (seek_plugin_handle.set_camera_setting == NULL) {
        printf("[Error] Issue while loading set_camera_setting in libeview_itf_seek.so, aborting \n");
        return EVIEWITF_FAIL;
    }
    seek_plugin_handle.get_camera_frame = dlsym(seek_plugin_handle.handle_plugin, "eviewitf_seek_get_camera_frame");
    if (seek_plugin_handle.get_camera_frame == NULL) {
        printf("[Error] Issue while loading get_camera_frame in libeview_itf_seek.so, aborting \n");
        return EVIEWITF_FAIL;
    }
    return EVIEWITF_OK;
}

/**
 * \fn eviewitf_seek_get_seek_version
 * \brief Get seek library version
 *
 * \return seek library version. Return NULL if fail
 */
char *eviewitf_seek_get_seek_version(void) {
    if (seek_version == NULL) {
        if (seek_plugin_handle.get_seek_lib_version != NULL) {
            seek_version = seek_plugin_handle.get_seek_lib_version();
        } else {
            printf("[Error] no reference to get_seek_lib_version \n");
            return NULL;
        }
    }
    return seek_version;
}

/**
 * \fn eviewitf_seek_get_plugin_version
 * \brief Get seek plugin library version
 *
 * \return seek plugin version. Return NULL if fail
 */
char *eviewitf_seek_get_plugin_version(void) {
    if (seek_plugin_version == NULL) {
        if (seek_plugin_handle.get_lib_version != NULL) {
            seek_plugin_version = seek_plugin_handle.get_lib_version();
        } else {
            printf("[Error] no reference to get_lib_version \n");
            return NULL;
        }
    }
    return seek_plugin_version;
}

/**
 * \fn eviewitf_seek_init_all_cameras
 * \brief Find the available cameras and open them.
 *
 * \param [in] nb_cam: Number of seek camera to initialize
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
int eviewitf_seek_init_all_cameras(int nb_cam) {
    int ret;

    if (seek_plugin_handle.init_all_cameras != NULL) {
        ret = seek_plugin_handle.init_all_cameras(nb_cam);
    } else {
        printf("[Error] no reference to init_all_cameras \n");
        return EVIEWITF_FAIL;
    }
    return ret;
}

/**
 * \fn eviewitf_seek_deinit_all_cameras
 * \brief Stop and close all the opened cameras.
 *
 * \param [in] nb_cam: Number of seek camera to de-initialize
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
int eviewitf_seek_deinit_all_cameras(int nb_cam) {
    int ret;

    if (seek_plugin_handle.deinit_all_cameras != NULL) {
        ret = seek_plugin_handle.deinit_all_cameras(nb_cam);
    } else {
        printf("[Error] no reference to deinit_all_cameras \n");
        return EVIEWITF_FAIL;
    }
    return ret;
}

/**
 * \fn eviewitf_seek_start_camera
 * \brief Start (if not already started) one camera and start its background processing.
 *
 * \param [in] cam_id: Id of the seek camera to start
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
int eviewitf_seek_start_camera(int cam_id) {
    int ret;

    if (seek_plugin_handle.start_camera != NULL) {
        ret = seek_plugin_handle.start_camera(cam_id);
    } else {
        printf("[Error] no reference to start_camera \n");
        return EVIEWITF_FAIL;
    }
    return ret;
}

/**
 * \fn eviewitf_seek_stop_camera
 * \brief Stop one camera and its background processing.
 *
 * \param [in] cam_id: Id of the seek camera to stop
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
int eviewitf_seek_stop_camera(int cam_id) {
    int ret;

    if (seek_plugin_handle.stop_camera != NULL) {
        ret = seek_plugin_handle.stop_camera(cam_id);
    } else {
        printf("[Error] no reference to stop_camera \n");
        return EVIEWITF_FAIL;
    }
    return ret;
}

/**
 * \fn eviewitf_seek_get_camera_setting
 * \brief Get one setting of one camera.
 *
 * \param [in] cam_id: Id of the seek camera to get a setting from
 * \param [in] setting_nb: Setting id to be read
 * \param [out] setting_value: Pointer to the address to store the read setting
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
int eviewitf_seek_get_camera_setting(int cam_id, int setting_nb, int *setting_value) {
    int ret;

    if (seek_plugin_handle.get_camera_setting != NULL) {
        ret = seek_plugin_handle.get_camera_setting(cam_id, setting_nb, setting_value);
    } else {
        printf("[Error] no reference to get_camera_setting \n");
        return EVIEWITF_FAIL;
    }
    return ret;
}

/**
 * \fn eviewitf_seek_set_camera_setting
 * \brief Set one setting of one camera.
 *
 * \param [in] cam_id: Id of the seek camera to set a setting to
 * \param [in] setting_nb: Setting id to be written
 * \param [in] setting_value: Value to be written
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
int eviewitf_seek_set_camera_setting(int cam_id, int setting_nb, int setting_value) {
    int ret;

    if (seek_plugin_handle.set_camera_setting != NULL) {
        ret = seek_plugin_handle.set_camera_setting(cam_id, setting_nb, setting_value);
    } else {
        printf("[Error] no reference to set_camera_setting \n");
        return EVIEWITF_FAIL;
    }
    return ret;
}

/**
 * \fn eviewitf_seek_get_camera_frame
 * \brief Get the latest frame of a Seek camera
 *
 * \param [in] cam_id: Id of the seek camera to get a frame from
 * \param [inout] temperature: Pointer to retrieve the temperature frame
 * \param [inout] display: Pointer to retrieve the display frame (ARGB8888 format)
 *
 * \return state of the function. Returns EVIEWITF_OK if okay
 */
int eviewitf_seek_get_camera_frame(int cam_id, float **temperature, uint32_t **display) {
    int ret;

    if (seek_plugin_handle.get_camera_frame != NULL) {
        ret = seek_plugin_handle.get_camera_frame(cam_id, temperature, display);
    } else {
        printf("[Error] no reference to get_camera_frame \n");
        return EVIEWITF_FAIL;
    }
    return ret;
}

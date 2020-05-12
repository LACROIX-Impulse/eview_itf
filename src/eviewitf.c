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
/* Structures used for internal communication between A53 and R7.
 Doesn't need to be exposed in API */
typedef struct {
    uint32_t buffer_size;
} eviewitf_cam_buffers_physical_r7_t;

typedef struct {
    eviewitf_cam_buffers_physical_r7_t cam[EVIEWITF_MAX_CAMERA];
    eviewitf_cam_buffers_physical_r7_t O2;
    eviewitf_cam_buffers_physical_r7_t O3;
} eviewitf_cam_buffers_r7_t;

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/
typedef enum {
    FCT_GET_CAM_BUFFERS,
    FCT_INIT_API,
    FCT_DEINIT_API,
    FCT_SET_DISPLAY_CAM,
    FCT_CAM_REG_R,
    FCT_CAM_REG_W,
    FCT_REBOOT_CAM,
    FCT_START_BLENDING,
    FCT_STOP_BLENDING,
    FCT_SET_FPS,
    FCT_HEARTBEAT,
    FCT_BOOT_MODE,
    FCT_GET_EVIEW_VERSION,
    FCT_UPDATE_CROPPING,
    NB_FCT,
} fct_id_t;

eviewitf_cam_buffers_a53_t *cam_virtual_buffers = NULL;
char eview_version[MAX_VERSION_SIZE];
/******************************************************************************************
 * Functions
 ******************************************************************************************/

/* Private function used to retrieved cam buffer during api initialization.
 Doesn't need to be exposed in API */
static int eviewitf_get_cam_buffers(eviewitf_cam_buffers_a53_t *virtual_buffers) {
    int ret = EVIEWITF_OK, i;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];
    eviewitf_cam_buffers_r7_t *cam_buffers_r7;

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_GET_CAM_BUFFERS;

    /* Check input parameter */
    if (virtual_buffers == NULL) {
        printf("Error eviewitf_get_cam_buffers called with null parameter\n");
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        /* Send request to R7 and check returned answer state */
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < 0) || (rx_buffer[0] != FCT_GET_CAM_BUFFERS) || (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
            return ret;
        }
    }
    cam_buffers_r7 =
        (eviewitf_cam_buffers_r7_t *)mfis_get_virtual_address(rx_buffer[2], sizeof(eviewitf_cam_buffers_r7_t));
    if (ret >= EVIEWITF_OK) {
        /* Fill the A53 cam_buffer structure with value returned by R7*/
        for (i = 0; i < EVIEWITF_MAX_CAMERA; i++) {
            virtual_buffers->cam[i].buffer_size = cam_buffers_r7->cam[i].buffer_size;
        }
        virtual_buffers->O2.buffer_size = cam_buffers_r7->O2.buffer_size;
        virtual_buffers->O3.buffer_size = cam_buffers_r7->O3.buffer_size;
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
    if (cam_virtual_buffers != NULL) {
        printf("eviewitf_init_api already done\n");
        ret = EVIEWITF_FAIL;
    } else {
        /* Prepare TX buffer */
        tx_buffer[0] = FCT_INIT_API;

        /* Send request to R7 and check returned answer state*/
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_INIT_API) || (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = EVIEWITF_FAIL;
        } else {
            // Get pointers to the cameras frame buffers located in R7 memory
            cam_virtual_buffers = malloc(sizeof(eviewitf_cam_buffers_a53_t));
            ret = eviewitf_get_cam_buffers(cam_virtual_buffers);
            for (int i = 0; i < EVIEWITF_MAX_CAMERA; i++) {
                if (cam_virtual_buffers->cam[i].buffer_size > 0) {
                    cam_virtual_buffers->cam[i].buffer = malloc(cam_virtual_buffers->cam[i].buffer_size);
                } else {
                    cam_virtual_buffers->cam[i].buffer = NULL;
                }
            }
            if (cam_virtual_buffers->O2.buffer_size > 0) {
                cam_virtual_buffers->O2.buffer = malloc(cam_virtual_buffers->O2.buffer_size);
            } else {
                cam_virtual_buffers->O2.buffer = NULL;
            }
            if (cam_virtual_buffers->O3.buffer_size > 0) {
                cam_virtual_buffers->O3.buffer = malloc(cam_virtual_buffers->O2.buffer_size);
            } else {
                cam_virtual_buffers->O3.buffer = NULL;
            }
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
    if (cam_virtual_buffers == NULL) {
        printf("eviewitf_init_api never done\n");
        ret = EVIEWITF_FAIL;
    } else {
        /* Free allocated resources */
        for (int i = 0; i < EVIEWITF_MAX_CAMERA; i++) {
            if (cam_virtual_buffers->cam[i].buffer != NULL) {
                free(cam_virtual_buffers->cam[i].buffer);
                cam_virtual_buffers->cam[i].buffer = NULL;
            }
        }
        free(cam_virtual_buffers);
        cam_virtual_buffers = NULL;

        /* Prepare TX buffer */
        tx_buffer[0] = FCT_DEINIT_API;

        /* Send request to R7 */
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_DEINIT_API) || (rx_buffer[1] != FCT_RETURN_OK)) {
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
    tx_buffer[0] = FCT_SET_DISPLAY_CAM;
    tx_buffer[1] = cam_id;
    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if (ret < EVIEWITF_OK) {
        ret = EVIEWITF_FAIL;
    } else {
        /* Check returned answer state */
        if ((rx_buffer[0] != FCT_SET_DISPLAY_CAM) && (rx_buffer[1] != FCT_RETURN_OK)) {
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
        ret = ssd_save_camera_stream(cam_id, delay, record_dir, cam_virtual_buffers->cam[cam_id].buffer_size);
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
int eviewitf_get_camera_param(int cam_id, int cam_type, int reg_adress, uint32_t *reg_value) {
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
        tx_buffer[0] = FCT_CAM_REG_R;
        tx_buffer[1] = cam_id;
        tx_buffer[2] = cam_type;
        tx_buffer[3] = reg_adress;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != FCT_CAM_REG_R) {
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
int eviewitf_set_camera_param(int cam_id, int cam_type, int reg_adress, uint32_t reg_value) {
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
        tx_buffer[0] = FCT_CAM_REG_W;
        tx_buffer[1] = cam_id;
        tx_buffer[2] = cam_type;
        tx_buffer[3] = reg_adress;
        tx_buffer[4] = (int32_t)reg_value;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != FCT_CAM_REG_W) {
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

        tx_buffer[0] = FCT_REBOOT_CAM;
        tx_buffer[1] = cam_id;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != FCT_REBOOT_CAM) {
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

    tx_buffer[0] = FCT_START_BLENDING;
    tx_buffer[1] = blending_id;
    ret = mfis_send_request(tx_buffer, rx_buffer);

    if (ret < EVIEWITF_OK) {
        ret = EVIEWITF_FAIL;
    } else {
        /* Check returned answer state */
        if (rx_buffer[0] != FCT_START_BLENDING) {
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

    tx_buffer[0] = FCT_STOP_BLENDING;
    ret = mfis_send_request(tx_buffer, rx_buffer);

    if (ret < EVIEWITF_OK) {
        ret = EVIEWITF_FAIL;
    } else {
        /* Check returned answer state */
        if (rx_buffer[0] != FCT_STOP_BLENDING) {
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
    int file_cam = 0;
    unsigned long int i;

    /* Test API has been initialized */
    if (cam_virtual_buffers == NULL) {
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
        ret = ssd_set_virtual_camera_stream(cam_id, cam_virtual_buffers->cam[i].buffer_size, fps, frames_dir);
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
    if (cam_virtual_buffers == NULL) {
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
    int file_cam = 0;

    /* Test API has been initialized */
    if (cam_virtual_buffers == NULL) {
        printf("Please call eviewitf_init_api first\n");
        ret = EVIEWITF_FAIL;
    }

    if (EVIEWITF_OK == ret) {
        switch (blending_id) {
            case 0:
                ret = ssd_set_blending(blending_id, cam_virtual_buffers->O2.buffer_size, frame);
                break;
            case 1:
                ret = ssd_set_blending(blending_id, cam_virtual_buffers->O3.buffer_size, frame);
                break;
            default:
                printf("Device %d not allowed for blending \n", blending_id);
                ret = EVIEWITF_INVALID_PARAM;
                break;
        }

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
    if (cam_virtual_buffers == NULL) {
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

        tx_buffer[0] = FCT_SET_FPS;
        tx_buffer[1] = cam_id;
        tx_buffer[2] = (int32_t)fps;
        ret = mfis_send_request(tx_buffer, rx_buffer);

        if (ret < EVIEWITF_OK) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Check returned answer state */
            if (rx_buffer[0] != FCT_SET_FPS) {
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
    tx_buffer[0] = FCT_HEARTBEAT;
    tx_buffer[1] = mode;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_HEARTBEAT) || (rx_buffer[1] != FCT_RETURN_OK)) {
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
    tx_buffer[0] = FCT_BOOT_MODE;
    tx_buffer[1] = mode;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_BOOT_MODE) || (rx_buffer[1] != FCT_RETURN_OK)) {
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
    int mem_dev;
    int i = 0;
    int j = 0;
    int size_div = 0;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];
    int32_t *ptr = NULL;
    char *tmp;

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
    tx_buffer[0] = FCT_UPDATE_CROPPING;
    tx_buffer[1] = x1;
    tx_buffer[2] = y1;
    tx_buffer[3] = x2;
    tx_buffer[4] = y2;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if ((ret < EVIEWITF_OK) || (rx_buffer[0] != FCT_UPDATE_CROPPING) || (rx_buffer[1] != FCT_RETURN_OK)) {
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

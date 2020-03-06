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
#include <poll.h>
#include "mfis_communication.h"
#include "eviewitf.h"
#include "eviewitf_ssd.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/
/* Magic number used to check metadata presence */
#define FRAME_MAGIC_NUMBER 0xD1CECA5F

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
} eviewitf_cam_buffers_r7_t;

/* Structures used for internal lib purpose.
   Doesn't need to be exposed in API */
typedef struct {
    uint32_t buffer_size;
    uint8_t* buffer;
} eviewitf_cam_buffers_virtual_t;

typedef struct {
    eviewitf_cam_buffers_virtual_t cam[EVIEWITF_MAX_CAMERA];
} eviewitf_cam_buffers_a53_t;

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
    FCT_VIRTUAL_CAM_UPDATE,
    FCT_SET_FPS,
    NB_FCT,
} fct_id_t;

typedef enum {
    FCT_RETURN_OK = 1,
    FCT_RETURN_BLOCKED,
    FCT_RETURN_ERROR,
    FCT_INV_PARAM,
} fct_ret_r;
static eviewitf_cam_buffers_a53_t* cam_virtual_buffers = NULL;

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/* Private function used to retrieved cam buffer during api initialization.
   Doesn't need to be exposed in API */
static int eviewitf_get_cam_buffers(eviewitf_cam_buffers_a53_t* virtual_buffers) {
    int ret = EVIEWITF_OK, i;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];
    eviewitf_cam_buffers_r7_t* cam_buffers_r7;

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
        (eviewitf_cam_buffers_r7_t*)mfis_get_virtual_address(rx_buffer[2], sizeof(eviewitf_cam_buffers_r7_t));
    if (ret >= EVIEWITF_OK) {
        /* Fill the A53 cam_buffer structure with value returned by R7*/
        for (i = 0; i < EVIEWITF_MAX_CAMERA; i++) {
            virtual_buffers->cam[i].buffer_size = cam_buffers_r7->cam[i].buffer_size;
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_get_frame(int cam_id, eviewitf_frame_buffer_info_t* frame_buffer, eviewitf_frame_metadata_info_t*
 frame_metadata)
 * \brief Return pointer to the camera frame buffer and to the associated metadata if any
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param eviewitf_frame_buffer_info_t* frame_buffer: structure that will be filled with frame buffer pointer and size
 * \param eviewitf_frame_metadata_info_t* frame_metadata: structure that will be filled with frame metadata pointer and
 size

 * \return state of the function. Return 0 if okay
 */
int eviewitf_get_frame(int cam_id, eviewitf_frame_buffer_info_t* frame_buffer,
                       eviewitf_frame_metadata_info_t* frame_metadata) {
    int ret = EVIEWITF_OK;
    int file_cam = 0;
    int cam_frame_id;
    uint8_t* ptr_metadata;
    eviewitf_frame_metadata_info_t* metadata = NULL;
    int ismetadata = 1;

    // Test API has been initialized
    if (cam_virtual_buffers == NULL) {
        printf("Please call eviewitf_init_api first\n");
        ret = EVIEWITF_FAIL;
    }
    // Test camera id
    else if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        printf("Invalid camera id\n");
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (ret >= EVIEWITF_OK) {
        // Get mfis device filename
        file_cam = open(mfis_device_filenames[cam_id], O_RDONLY);
        if (file_cam == -1) {
            printf("Error opening camera file\n");
            ret = EVIEWITF_FAIL;
        }
    }

    if (ret >= EVIEWITF_OK) {
        // Read file content
        ret = read(file_cam, cam_virtual_buffers->cam[cam_id].buffer, cam_virtual_buffers->cam[cam_id].buffer_size);
        if (ret < EVIEWITF_OK) {
            printf("Error reading camera file\n");
            ret = EVIEWITF_FAIL;
        }
    }

    if (ret >= EVIEWITF_OK) {
        // Metadata magic number is located at the end of the buffer if present
        ptr_metadata = cam_virtual_buffers->cam[cam_id].buffer + cam_virtual_buffers->cam[cam_id].buffer_size -
                       sizeof(eviewitf_frame_metadata_info_t);
        metadata = (eviewitf_frame_metadata_info_t*)ptr_metadata;
        if (metadata->magic_number == FRAME_MAGIC_NUMBER) {
            if (metadata->frame_size > cam_virtual_buffers->cam[cam_id].buffer_size) {
                // Special case where frame's data looks like a magic number
                ismetadata = 0;
            } else {
                if ((metadata->frame_width * metadata->frame_height * metadata->frame_bpp) != metadata->frame_size) {
                    // Special case where:
                    // - frame's data looks like a magic number
                    // - frame_size is lower than buffer_size
                    ismetadata = 0;
                } else {
                    // Metadata are present and valid
                    if (frame_metadata != NULL) {
                        frame_metadata->frame_width = metadata->frame_width;
                        frame_metadata->frame_height = metadata->frame_height;
                        frame_metadata->frame_bpp = metadata->frame_bpp;
                        frame_metadata->frame_timestamp_lsb = metadata->frame_timestamp_lsb;
                        frame_metadata->frame_timestamp_msb = metadata->frame_timestamp_msb;
                        frame_metadata->frame_sync = metadata->frame_sync;
                    }
                    if (frame_buffer != NULL) {
                        frame_buffer->buffer_size = metadata->frame_size;
                        frame_buffer->ptr_buf = cam_virtual_buffers->cam[cam_id].buffer;
                    }
                }
            }
        } else {
            // Magic number not found, no metadata
            ismetadata = 0;
        }

        if (ismetadata == 0) {
            // No metadata available
            if (frame_metadata != NULL) {
                frame_metadata->frame_width = 0;
                frame_metadata->frame_height = 0;
                frame_metadata->frame_bpp = 0;
                frame_metadata->frame_timestamp_lsb = 0;
                frame_metadata->frame_timestamp_msb = 0;
            }
            if (frame_buffer != NULL) {
                frame_buffer->buffer_size = cam_virtual_buffers->cam[cam_id].buffer_size;
                frame_buffer->ptr_buf = cam_virtual_buffers->cam[cam_id].buffer;
            }
        }
    }

    close(file_cam);
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
    char* record_dir = NULL;

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
int eviewitf_get_camera_param(int cam_id, int cam_type, int reg_adress, uint16_t* reg_value) {
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
        *reg_value = (uint16_t)rx_buffer[2];
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
int eviewitf_set_camera_param(int cam_id, int cam_type, int reg_adress, int reg_value) {
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
        tx_buffer[4] = reg_value;
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
 * \fn eviewitf_virtual_cam_update
 * \brief Update the frames to be printed on a virtual camera

 * \param in cam_id: id of the camera
 * \param in fps: fps to apply on the recording
 * \param in frames_dir: path to the recording
 * \return state of the function. Return 0 if okay
 */
int eviewitf_virtual_cam_update(int cam_id, int fps, char* frames_dir) {
    int ret = EVIEWITF_OK;
    int file_cam = 0;
    unsigned long int i;
    ssize_t ret_write = 0;

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

int eviewitf_poll(int* cam_id, int nb_cam, short* event_return) {
    short revents;
    struct pollfd pfd[nb_cam];
    int r_poll;
    int file_cam[nb_cam];
    int ret = EVIEWITF_OK;
    int i;

    for (i = 0; i < nb_cam; i++) {
        if ((cam_id[i] < 0) || (cam_id[i] >= EVIEWITF_MAX_CAMERA)) {
            printf("Invalid camera id %d\n", cam_id[i]);
            ret = EVIEWITF_INVALID_PARAM;
        }
    }
    for (i = 0; i < nb_cam; i++) {
        if (ret >= EVIEWITF_OK) {
            // Get mfis device filename
            file_cam[i] = open(mfis_device_filenames[cam_id[i]], O_RDONLY);
            if (file_cam[i] == -1) {
                printf("Error opening camera file %d\n", cam_id[i]);
                ret = EVIEWITF_FAIL;
            }
        }
        pfd[i].fd = file_cam[i];
        pfd[i].events = POLLIN;
    }

    if (ret >= EVIEWITF_OK) {
        r_poll = poll(pfd, nb_cam, -1);
        if (r_poll == -1) {
            printf("POLL ERROR \n");
            ret = EVIEWITF_FAIL;
        }
    }

    for (i = 0; i < nb_cam; i++) {
        if (ret >= EVIEWITF_OK) {
            event_return[i] = pfd[i].revents;
            close(file_cam[i]);
        }
    }
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

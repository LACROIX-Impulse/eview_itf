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
#include "mfis_communication.h"
#include "eviewitf.h"
#include "eviewitf_ssd.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/
/* Function returned state */
#define FCT_RETURN_OK 1
/* Magic number used to check metadata presence */
#define FRAME_MAGIC_NUMBER 0xD1CECA5E

/******************************************************************************************
 * Private structures
 ******************************************************************************************/
/* Structures used for internal communication between A53 and R7.
   Doesn't need to be exposed in API */
typedef struct {
    uint32_t buffer_size;
    uint32_t ptr_buf[3];
} eviewitf_cam_buffers_physical_r7_t;

typedef struct {
    eviewitf_cam_buffers_physical_r7_t cam[EVIEWITF_MAX_CAMERA];
} eviewitf_cam_buffers_r7_t;

/* Structures used for internal lib purpose.
   Doesn't need to be exposed in API */
typedef struct {
    uint32_t buffer_size;
    uint8_t* ptr_buf[3];
} eviewitf_cam_buffers_virtual_t;

typedef struct {
    eviewitf_cam_buffers_virtual_t cam[EVIEWITF_MAX_CAMERA];
} eviewitf_cam_buffers_a53_t;

typedef struct {
    uint32_t frame_size;
    uint32_t magic_number;
} eviewitf_frame_metadata_t;

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/
typedef enum {
    FCT_GET_CAM_BUFFERS,
    FCT_INIT_API,
    FCT_DEINIT_API,
    FCT_SET_DISPLAY_CAM,
    NB_FCT,
} fct_id_t;

static eviewitf_cam_buffers_a53_t* cam_virtual_buffers = NULL;
static const char* mfis_device_filenames[EVIEWITF_MAX_CAMERA] = {"/dev/mfis_cam0", "/dev/mfis_cam1", "/dev/mfis_cam2",
                                                                 "/dev/mfis_cam3", "/dev/mfis_cam4", "/dev/mfis_cam5",
                                                                 "/dev/mfis_cam6", "/dev/mfis_cam7"};

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/* Private function used to retrieved cam buffer during api initialization.
   Doesn't need to be exposed in API */
static int eviewitf_get_cam_buffers(eviewitf_cam_buffers_a53_t* virtual_buffers) {
    int ret = 0, i;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];
    eviewitf_cam_buffers_r7_t* cam_buffers_r7;

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_GET_CAM_BUFFERS;

    /* Check input parameter */
    if (virtual_buffers == NULL) {
        printf("Error eviewitf_get_cam_buffers called with null parameter\n");
        ret = -1;
    } else {
        /* Send request to R7 and check returned answer state */
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < 0) || (rx_buffer[0] != FCT_GET_CAM_BUFFERS) || (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = -1;
        }
    }

    if (ret >= 0) {
        /* R7 return a pointer to a structure stored in its memory. Convert this pointer into a virtual adress for A53
         */
        cam_buffers_r7 =
            (eviewitf_cam_buffers_r7_t*)mfis_get_virtual_address(rx_buffer[2], sizeof(eviewitf_cam_buffers_r7_t));

        /* Fill the A53 cam_buffer structure with value returned by R7*/
        for (i = 0; i < EVIEWITF_MAX_CAMERA; i++) {
            virtual_buffers->cam[i].buffer_size = cam_buffers_r7->cam[i].buffer_size;

            if (virtual_buffers->cam[i].buffer_size != 0) {
                /* Convert R7 physical addresses of frame buffers to virtual adresses */
                virtual_buffers->cam[i].ptr_buf[0] = (uint8_t*)mfis_get_virtual_address(
                    cam_buffers_r7->cam[i].ptr_buf[0], cam_buffers_r7->cam[i].buffer_size);
                virtual_buffers->cam[i].ptr_buf[1] = (uint8_t*)mfis_get_virtual_address(
                    cam_buffers_r7->cam[i].ptr_buf[1], cam_buffers_r7->cam[i].buffer_size);
                virtual_buffers->cam[i].ptr_buf[2] = (uint8_t*)mfis_get_virtual_address(
                    cam_buffers_r7->cam[i].ptr_buf[2], cam_buffers_r7->cam[i].buffer_size);
            }
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
    int ret = 0;
    int file_cam = 0;
    int cam_frame_id;
    char cam_read_param[2];
    uint8_t* ptr_metadata;
    eviewitf_frame_metadata_t* metadata = NULL;
    eviewitf_frame_metadata_info_t* metadata_info = NULL;
    int ismetadata = 1;

    // Test API has been initialized
    if (cam_virtual_buffers == NULL) {
        printf("Please call eviewitf_init_api first\n");
        ret = -1;
    }
    // Test camera id
    else if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        printf("Invalid camera id\n");
        ret = -1;
    }

    if (ret >= 0) {
        // Get mfis device filename
        file_cam = open(mfis_device_filenames[cam_id], O_RDONLY);
        if (file_cam == -1) {
            printf("Error opening camera file\n");
            ret = -1;
        }
    }
    if (ret >= 0) {
        // Read file content
        ret = read(file_cam, &cam_read_param, 2);
        if (ret < 0) {
            printf("Error reading camera file\n");
            ret = -1;
        }
    }

    if (ret >= 0) {
        cam_frame_id = cam_read_param[0];
        // Metadata magic number is located at the end of the buffer if present
        ptr_metadata = cam_virtual_buffers->cam[cam_id].ptr_buf[cam_frame_id] +
                       cam_virtual_buffers->cam[cam_id].buffer_size - sizeof(eviewitf_frame_metadata_t);
        metadata = (eviewitf_frame_metadata_t*)ptr_metadata;
        if (metadata->magic_number == FRAME_MAGIC_NUMBER) {
            if (metadata->frame_size > cam_virtual_buffers->cam[cam_id].buffer_size) {
                // Special case where frame's data looks like a magic number
                ismetadata = 0;
            } else {
                ptr_metadata = cam_virtual_buffers->cam[cam_id].ptr_buf[cam_frame_id] + metadata->frame_size;
                metadata_info = (eviewitf_frame_metadata_info_t*)ptr_metadata;
                if ((metadata_info->frame_width * metadata_info->frame_height * metadata_info->frame_bpp) !=
                    metadata->frame_size) {
                    // Special case where:
                    // - frame's data looks like a magic number
                    // - frame_size is lower than buffer_size
                    ismetadata = 0;
                } else {
                    // Metadata are present and valid
                    if (frame_metadata != NULL) {
                        frame_metadata->frame_width = metadata_info->frame_width;
                        frame_metadata->frame_height = metadata_info->frame_height;
                        frame_metadata->frame_bpp = metadata_info->frame_bpp;
                        frame_metadata->frame_timestamp_lsb = metadata_info->frame_timestamp_lsb;
                        frame_metadata->frame_timestamp_msb = metadata_info->frame_timestamp_msb;
                    }
                    if (frame_buffer != NULL) {
                        frame_buffer->buffer_size = metadata->frame_size;
                        frame_buffer->ptr_buf = cam_virtual_buffers->cam[cam_id].ptr_buf[cam_frame_id];
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
                frame_buffer->ptr_buf = cam_virtual_buffers->cam[cam_id].ptr_buf[cam_frame_id];
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
    int ret = 0;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Check if init has been done */
    if (cam_virtual_buffers != NULL) {
        printf("eviewitf_init_api already done\n");
        ret = -1;
    } else {
        /* Prepare TX buffer */
        tx_buffer[0] = FCT_INIT_API;

        /* Send request to R7 and check returned answer state*/
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < 0) || (rx_buffer[0] != FCT_INIT_API) || (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = -1;
            printf("Error in eviewitf_init_api\n");
        } else {
            // Get pointers to the cameras frame buffers located in R7 memory
            cam_virtual_buffers = malloc(sizeof(eviewitf_cam_buffers_a53_t));
            ret = eviewitf_get_cam_buffers(cam_virtual_buffers);
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
    int ret = 0;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Check if init has been done */
    if (cam_virtual_buffers == NULL) {
        printf("eviewitf_init_api never done\n");
        ret = -1;
    } else {
        /* Free allocated resources */
        free(cam_virtual_buffers);
        cam_virtual_buffers = NULL;

        /* Prepare TX buffer */
        tx_buffer[0] = FCT_DEINIT_API;

        /* Send request to R7 */
        ret = mfis_send_request(tx_buffer, rx_buffer);
        if ((ret < 0) || (rx_buffer[0] != FCT_DEINIT_API) || (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = -1;
        }
    }

    return ret;
}

/**
 * \fn eviewitf_set_display_cam
 * \brief Request R7 to change camera used on display
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_set_display_cam(int cam_id) {
    int ret = 0;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_SET_DISPLAY_CAM;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if (ret < 0) {
        ret = -1;
    } else {
        /* Check returned answer state */
        if ((rx_buffer[0] != FCT_SET_DISPLAY_CAM) && (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = -1;
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
    int ret;
    char* record_dir = NULL;
    ssd_get_output_directory(&record_dir);
    printf("SSD storage directory %s \n", record_dir);
    ret = ssd_save_camera_stream(cam_id, delay, record_dir);
    free(record_dir);
    return ret;
}

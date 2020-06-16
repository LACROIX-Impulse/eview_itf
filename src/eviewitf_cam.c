/**
 * \file eviewitf-cam.c
 * \brief Communication API between A53 and R7 CPUs for camera devices
 * \author eSoftThings
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>
#include "eviewitf.h"
#include "eviewitf_priv.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/
/* Magic number used to check metadata presence */
#define FRAME_MAGIC_NUMBER 0xD1CECA5F

/******************************************************************************************
 * Private structures
 ******************************************************************************************/

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/
extern eviewitf_cam_buffers_a53_t *cam_virtual_buffers;

/******************************************************************************************
 * Private variables
 ******************************************************************************************/
static int file_cams[EVIEWITF_MAX_CAMERA] = {-1};

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/**
 * \fn int eviewitf_camera_open(int cam_id)
 * \brief Open a camera device
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA

 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_open(int cam_id) {
    int ret = EVIEWITF_OK;

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
        file_cams[cam_id] = open(mfis_device_filenames[cam_id], O_RDONLY);
        if (file_cams[cam_id] == -1) {
            printf("Error opening camera file\n");
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_camera_close(int cam_id)
 * \brief Close a camera device
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA

 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_close(int cam_id) {
    int ret = EVIEWITF_OK;

    // Test camera id
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        printf("Invalid camera id\n");
        ret = EVIEWITF_INVALID_PARAM;
    }
    // Test camera has been opened
    if (file_cams[cam_id] == -1) {
        printf("Camera is not open\n");
        ret = EVIEWITF_FAIL;
    }

    if (ret >= EVIEWITF_OK) {
        // Get mfis device filename
        if (close(file_cams[cam_id] != 0)) {
            printf("Error closing camera file\n");
            ret = EVIEWITF_FAIL;
        } else {
            file_cams[cam_id] = -1;
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_camera_get_frame(int cam_id, uint8_t *frame_buffer, uint32_t buffer_size)
 * \brief Copy frame from physical memory to the given buffer location
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param frame_buffer: buffer to store the incoming frame
 * \param buffer_size: buffer size for coherency check

 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_get_frame(int cam_id, uint8_t *frame_buffer, uint32_t buffer_size) {
    int ret = EVIEWITF_OK;

    // Test camera id
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        printf("Invalid camera id\n");
        ret = EVIEWITF_INVALID_PARAM;
    } else if (file_cams[cam_id] == -1) {
        printf("Camera is not open\n");
        ret = EVIEWITF_FAIL;
    }

    if (ret >= EVIEWITF_OK) {
        // Read file content
        ret = read(file_cams[cam_id], frame_buffer, buffer_size);
        if (ret < EVIEWITF_OK) {
            printf("Error reading camera file\n");
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_poll(int *cam_id, int nb_cam, short *event_return)
 * \brief Poll on multiple cameras to check a new frame is available
 *
 * \param cam_id: table of camera ids to poll on (id between 0 and EVIEWITF_MAX_CAMERA)
 * \param nb_cam: number of cameras on which the polling applies
 * \param event_return: detected events for each camera, 0 if no frame, 1 if a frame is available

 * \return state of the function. Return 0 if okay
 */
int eviewitf_poll(int *cam_id, int nb_cam, short *event_return) {
    struct pollfd pfd[nb_cam];
    int r_poll;
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
            if (file_cams[cam_id[i]] == -1) {
                printf("Camera is not open\n");
                ret = EVIEWITF_FAIL;
            }
            pfd[i].fd = file_cams[cam_id[i]];
            pfd[i].events = POLLIN;
        }
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
            event_return[i] = pfd[i].revents & POLLIN;
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_check_camera_on(int cam_id)
 * \brief Check if the camera is available
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA

 * \return state of the function. Return 0 if okay
 */
int eviewitf_check_camera_on(int cam_id) {
    if (cam_virtual_buffers == NULL) {
        printf("eviewitf_init_api never done\n");
        return EVIEWITF_FAIL;
    } else if (cam_id < 0 || cam_id >= EVIEWITF_MAX_CAMERA) {
        printf("Invalid camera id %d\n", cam_id);
        return EVIEWITF_FAIL;
    } else {
        if (cam_virtual_buffers->cam[cam_id].buffer_size == 0) {
            return EVIEWITF_FAIL;
        } else {
            return EVIEWITF_OK;
        }
    }
}

/**
 * \fn int eviewitf_camera_get_buffer_size(int cam_id)
 * \brief Check the buffer size to be allocated to read frames
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA

 * \return size of the buffer. Return 0 if camera not available
 */
uint32_t eviewitf_camera_get_buffer_size(int cam_id) {
    if (cam_virtual_buffers == NULL) {
        printf("eviewitf_init_api never done\n");
        return 0;
    } else if (cam_id < 0 || cam_id >= EVIEWITF_MAX_CAMERA) {
        printf("Invalid camera id %d\n", cam_id);
        return 0;
    }

    return cam_virtual_buffers->cam[cam_id].buffer_size;
}

/**
 * \fn int eviewitf_camera_extract_metadata(uint8_t *buf, uint32_t buffer_size,
                              eviewitf_frame_metadata_info_t *frame_metadata)
 * \brief Extract metadata from a frame buffer
 *
 * \param buf: pointer on the buffe rwhere the frame is stored
 * \param buffer_size: size of the buffer
 * \param frame_metadata: pointer on metadata structure to be filled

 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_extract_metadata(uint8_t *buf, uint32_t buffer_size,
                                     eviewitf_frame_metadata_info_t *frame_metadata) {
    uint8_t *ptr_metadata;
    eviewitf_frame_metadata_info_t *metadata = NULL;
    int ismetadata = 1;

    if ((buf == NULL) || (frame_metadata == NULL)) {
        printf("Invalid buffer to extract metadata\n");
        return EVIEWITF_INVALID_PARAM;
    }

    if (buffer_size < sizeof(eviewitf_frame_metadata_info_t)) {
        printf("Invalid size to extract metadata\n");
        return EVIEWITF_INVALID_PARAM;
    }

    // Metadata magic number is located at the end of the buffer if present
    ptr_metadata = buf + buffer_size - sizeof(eviewitf_frame_metadata_info_t);
    metadata = (eviewitf_frame_metadata_info_t *)ptr_metadata;
    if (metadata->magic_number == FRAME_MAGIC_NUMBER) {
        if (metadata->frame_size > buffer_size) {
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
    }

    return EVIEWITF_OK;
}

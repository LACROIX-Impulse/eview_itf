/**
 * \file eviewitf_cam.c
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
#include "mfis_communication.h"

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
 * Private variables
 ******************************************************************************************/
static int file_cams[EVIEWITF_MAX_CAMERA] = {-1, -1, -1, -1, -1, -1, -1, -1};

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
    char device_name[DEVICE_CAMERA_MAX_LENGTH];
    struct eviewitf_mfis_camera_attributes *camera_attributes;

    /* Test API has been initialized */
    if (eviewitf_is_initialized() == 0) {
        ret = EVIEWITF_NOT_INITIALIZED;
    }

    /* Test camera id */
    else if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    /* Test already open */
    else if (file_cams[cam_id] != -1) {
        ret = EVIEWITF_FAIL;
    }

    /* Test camera is active */
    else {
        camera_attributes = eviewitf_get_camera_attributes(cam_id);
        if (camera_attributes->cam_type == EVIEWITF_MFIS_CAM_TYPE_NONE) {
            ret = EVIEWITF_FAIL;
        }
    }

    if (ret >= EVIEWITF_OK) {
        /* Get mfis device filename */
        snprintf(device_name, DEVICE_CAMERA_MAX_LENGTH, DEVICE_CAMERA_NAME, cam_id);
        file_cams[cam_id] = open(device_name, O_RDONLY);
        if (file_cams[cam_id] == -1) {
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

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (ret >= EVIEWITF_OK) {
        // Test camera has been opened
        if (file_cams[cam_id] == -1) {
            ret = EVIEWITF_NOT_OPENED;
        }
    }

    if (ret >= EVIEWITF_OK) {
        /* Get mfis device filename */
        if (close(file_cams[cam_id]) != 0) {
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

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else if (frame_buffer == NULL) {
        ret = EVIEWITF_INVALID_PARAM;
    } else if (file_cams[cam_id] == -1) {
        ret = EVIEWITF_NOT_OPENED;
    }

    if (ret >= EVIEWITF_OK) {
        /* Read file content */
        if (read(file_cams[cam_id], frame_buffer, buffer_size) < 0) {
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_camera_poll(int *cam_id, int nb_cam, short *event_return)
 * \brief Poll on multiple cameras to check a new frame is available
 *
 * \param cam_id: table of camera ids to poll on (id between 0 and EVIEWITF_MAX_CAMERA)
 * \param nb_cam: number of cameras on which the polling applies
 * \param event_return: detected events for each camera, 0 if no frame, 1 if a frame is available

 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_poll(int *cam_id, int nb_cam, short *event_return) {
    struct pollfd pfd[nb_cam];
    int r_poll;
    int ret = EVIEWITF_OK;
    int i;

    if ((cam_id == NULL) || (event_return == NULL)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        for (i = 0; i < nb_cam; i++) {
            if ((cam_id[i] < 0) || (cam_id[i] >= EVIEWITF_MAX_CAMERA)) {
                ret = EVIEWITF_INVALID_PARAM;
            }
        }
    }
    for (i = 0; i < nb_cam; i++) {
        if (ret >= EVIEWITF_OK) {
            if (file_cams[cam_id[i]] == -1) {
                ret = EVIEWITF_NOT_OPENED;
            }
            pfd[i].fd = file_cams[cam_id[i]];
            pfd[i].events = POLLIN;
        }
    }

    if (ret >= EVIEWITF_OK) {
        r_poll = poll(pfd, nb_cam, -1);
        if (r_poll == -1) {
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
 * \fn int eviewitf_camera_get_attributes(int cam_id)
 * \brief Get camera attributes such as buffer size
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param attributes: pointer on the structure to be filled

 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_get_attributes(int cam_id, eviewitf_device_attributes_t *attributes) {
    int ret = EVIEWITF_OK;
    /* Get the camera attributes */
    struct eviewitf_mfis_camera_attributes *camera_attributes = eviewitf_get_camera_attributes(cam_id);

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    /* Test attributes */
    if (ret >= EVIEWITF_OK) {
        if (attributes == NULL) {
            ret = EVIEWITF_INVALID_PARAM;
        }
    }

    /* Test API has been initialized */
    if (ret >= EVIEWITF_OK) {
        if (eviewitf_is_initialized() == 0) {
            ret = EVIEWITF_NOT_INITIALIZED;
        }
    }

    /* Copy attributes */
    if (ret >= EVIEWITF_OK) {
        attributes->buffer_size = camera_attributes->buffer_size;
        attributes->width = camera_attributes->width;
        attributes->height = camera_attributes->height;
        attributes->dt = camera_attributes->dt;
    }

    return ret;
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
    int ret = EVIEWITF_OK;
    uint8_t *ptr_metadata;
    eviewitf_frame_metadata_info_t *metadata = NULL;

    if ((buf == NULL) || (frame_metadata == NULL)) {
        return EVIEWITF_INVALID_PARAM;
    }

    if (buffer_size < sizeof(eviewitf_frame_metadata_info_t)) {
        return EVIEWITF_INVALID_PARAM;
    }

    /* Metadata magic number is located at the end of the buffer if present */
    ptr_metadata = buf + buffer_size - sizeof(eviewitf_frame_metadata_info_t);
    metadata = (eviewitf_frame_metadata_info_t *)ptr_metadata;
    if (metadata->magic_number == FRAME_MAGIC_NUMBER) {
        if (metadata->frame_size > buffer_size) {
            /* Special case where frame's data looks like a magic number */
            ret = EVIEWITF_FAIL;
        } else {
            if ((metadata->frame_width * metadata->frame_height * metadata->frame_bpp) != metadata->frame_size) {
                /* Special case where:                      */
                /* - frame's data looks like a magic number */
                /* - frame_size is lower than buffer_size   */
                ret = EVIEWITF_FAIL;
            } else {
                /* Metadata are present and valid */
                if (frame_metadata != NULL) {
                    frame_metadata->frame_width = metadata->frame_width;
                    frame_metadata->frame_height = metadata->frame_height;
                    frame_metadata->frame_bpp = metadata->frame_bpp;
                    frame_metadata->frame_timestamp_lsb = metadata->frame_timestamp_lsb;
                    frame_metadata->frame_timestamp_msb = metadata->frame_timestamp_msb;
                    frame_metadata->frame_sync = metadata->frame_sync;
                    frame_metadata->frame_size = metadata->frame_size;
                    frame_metadata->magic_number = metadata->magic_number;
                }
            }
        }
    } else {
        /* Magic number not found, no metadata */
        ret = EVIEWITF_FAIL;
    }

    if (ret != EVIEWITF_OK) {
        /* No metadata available */
        if (frame_metadata != NULL) {
            frame_metadata->frame_width = 0;
            frame_metadata->frame_height = 0;
            frame_metadata->frame_bpp = 0;
            frame_metadata->frame_timestamp_lsb = 0;
            frame_metadata->frame_timestamp_msb = 0;
            frame_metadata->frame_sync = 0;
            frame_metadata->frame_size = 0;
            frame_metadata->magic_number = 0;
        }
    }

    return ret;
}

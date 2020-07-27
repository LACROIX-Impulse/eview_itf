/**
 * \file eviewitf_streamer.c
 * \brief Communication API between A53 and R7 CPUs for streamer devices
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
static int file_streamers[EVIEWITF_MAX_STREAMER] = {-1, -1, -1, -1, -1, -1, -1, -1};

/******************************************************************************************
 * Functions
 ******************************************************************************************/

int camera_streamer_open(int cam_id) {
    char device_name[DEVICE_CAMERA_MAX_LENGTH];

    /* Get mfis device filename */
    snprintf(device_name, DEVICE_CAMERA_MAX_LENGTH, DEVICE_CAMERA_NAME, cam_id);
    return open(device_name, O_WRONLY);
}

int camera_streamer_close(int file_descriptor) {
    return close(file_descriptor);
}

int camera_streamer_write(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size) {
    return write(file_descriptor, frame_buffer, buffer_size);
}

/**
 * \fn int eviewitf_streamer_open(int streamer_id)
 * \brief Open a streamer device
 *
 * \param streamer_id: id of the streamer between 0 and EVIEWITF_MAX_STREAMER

 * \return state of the function. Return 0 if okay
 */
int eviewitf_streamer_open(int streamer_id) {
    int ret = EVIEWITF_OK;
    struct eviewitf_camera_object *streamer_object;

    /* Test API has been initialized */
    if (eviewitf_is_initialized() == 0) {
        ret = EVIEWITF_NOT_INITIALIZED;
    }

    /* Test streamer id */
    else if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    /* Test already open */
    else if (file_streamers[streamer_id] != -1) {
        ret = EVIEWITF_FAIL;
    }

    /* Test streamer is active */
    else {
        streamer_object = eviewitf_get_camera_object(streamer_id + EVIEWITF_MAX_CAMERA);
        if (streamer_object->camera_operations.open == NULL) {
            ret = EVIEWITF_FAIL;
        }
        else {
            file_streamers[streamer_id] = streamer_object->camera_operations.open(streamer_id + EVIEWITF_MAX_CAMERA);
            if (file_streamers[streamer_id] == -1) {
                return EVIEWITF_FAIL;
            }
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_streamer_close(int streamer_id)
 * \brief Close a streamer device
 *
 * \param streamer_id: id of the streamer between 0 and EVIEWITF_MAX_STREAMER

 * \return state of the function. Return 0 if okay
 */
int eviewitf_streamer_close(int streamer_id) {
    int ret = EVIEWITF_OK;
    struct eviewitf_camera_object *streamer_object;

    /* Test streamer id */
    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (ret >= EVIEWITF_OK) {
        // Test streamer has been opened
        if (file_streamers[streamer_id] == -1) {
            ret = EVIEWITF_NOT_OPENED;
        }
    }

    if (ret >= EVIEWITF_OK) {
        streamer_object = eviewitf_get_camera_object(streamer_id + EVIEWITF_MAX_CAMERA);
        if (streamer_object->camera_operations.close == NULL) {
            ret = EVIEWITF_FAIL;
        }
        else {
            if(streamer_object->camera_operations.close(file_streamers[streamer_id]) != 0) {;
                ret = EVIEWITF_FAIL;
            } else {
                file_streamers[streamer_id] = -1;
            }
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_streamer_get_attributes(int streamer_id)
 * \brief Get streamer attrubutes such as buffer size
 *
 * \param streamer_id: id of the streamer between 0 and EVIEWITF_MAX_STREAMER
 * \param attributes: pointer on the structure to be filled

 * \return state of the function. Return 0 if okay
 */
int eviewitf_streamer_get_attributes(int streamer_id, eviewitf_device_attributes_t *attributes) {
    int ret = EVIEWITF_OK;
    /* Get the streamers attributes */
    struct eviewitf_camera_object *streamer_object =
        eviewitf_get_camera_object(streamer_id + EVIEWITF_MAX_CAMERA);

    /* Test streamer id */
    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
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
        attributes->buffer_size = streamer_object->camera_attributes.buffer_size;
        attributes->width = streamer_object->camera_attributes.width;
        attributes->height = streamer_object->camera_attributes.height;
        attributes->dt = streamer_object->camera_attributes.dt;
    }

    return ret;
}

/**
 * \fn eviewitf_streamer_write_frame
 * \brief Write a frame to a streamer

 * \param in streamer_id: id of the camera
 * \param in buffer_size: size of the virtual camera buffer
 * \param in buffer: virtual camera buffer
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_streamer_write_frame(int streamer_id, uint8_t *frame_buffer, uint32_t buffer_size) {
    int ret = EVIEWITF_OK;
    /* Get the streamers attributes */
    struct eviewitf_camera_object *streamer_object;

    /* Test API has been initialized */
    if (eviewitf_is_initialized() == 0) {
        ret = EVIEWITF_NOT_INITIALIZED;
    }

    if (ret >= EVIEWITF_OK) {
        if (frame_buffer == NULL) {
            ret = EVIEWITF_INVALID_PARAM;
        }
    }

    if (ret >= EVIEWITF_OK) {
        // Test streamer has been opened
        if (file_streamers[streamer_id] == -1) {
            ret = EVIEWITF_NOT_OPENED;
        }
    }

    if (ret >= EVIEWITF_OK) {
        /* Write the frame in the virtual camera */
        streamer_object = eviewitf_get_camera_object(streamer_id + EVIEWITF_MAX_CAMERA);
        if (streamer_object->camera_operations.write == NULL) {
            ret = EVIEWITF_FAIL;
        }
        else {
            if(streamer_object->camera_operations.write(file_streamers[streamer_id], frame_buffer, buffer_size ) < 0) {
                ret = EVIEWITF_FAIL;
            }
        }
    }

    return ret;
}
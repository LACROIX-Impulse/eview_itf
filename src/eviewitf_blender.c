/**
 * \file eviewitf_blender.c
 * \brief Communication API between A53 and R7 CPUs for blender devices
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
static int file_blenders[EVIEWITF_MAX_BLENDER] = {-1};

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/**
 * \fn int eviewitf_blender_open(int blender_id)
 * \brief Open a blender device
 *
 * \param blender_id: id of the blender between 0 and EVIEWITF_MAX_CAMERA

 * \return state of the function. Return 0 if okay
 */
int eviewitf_blender_open(int blender_id) {
    int ret = EVIEWITF_OK;
    char device_name[DEVICE_BLENDER_MAX_LENGTH];

    /* Test API has been initialized */
    if (eviewitf_is_initialized() == 0) {
        ret = EVIEWITF_NOT_INITIALIZED;
    }

    /* Test blender id */
    else if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    /* Test already open */
    else if (file_blenders[blender_id] != -1) {
        ret = EVIEWITF_FAIL;
    }

    if (ret >= EVIEWITF_OK) {
        /* Get mfis device filename */
        snprintf(device_name, DEVICE_BLENDER_MAX_LENGTH, DEVICE_BLENDER_NAME, blender_id + 2); /* O2 and O3 */
        file_blenders[blender_id] = open(device_name, O_WRONLY);
        if (file_blenders[blender_id] == -1) {
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_blender_close(int blender_id)
 * \brief Close a blender device
 *
 * \param blender_id: id of the blender between 0 and EVIEWITF_MAX_CAMERA

 * \return state of the function. Return 0 if okay
 */
int eviewitf_blender_close(int blender_id) {
    int ret = EVIEWITF_OK;

    /* Test blender id */
    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    if (ret >= EVIEWITF_OK) {
        // Test blender has been opened
        if (file_blenders[blender_id] == -1) {
            ret = EVIEWITF_NOT_OPENED;
        }
    }

    if (ret >= EVIEWITF_OK) {
        /* Get mfis device filename */
        if (close(file_blenders[blender_id]) != 0) {
            ret = EVIEWITF_FAIL;
        } else {
            file_blenders[blender_id] = -1;
        }
    }

    return ret;
}

/**
 * \fn int eviewitf_blender_get_attributes(int blender_id)
 * \brief Get blender attrubutes such as buffer size
 *
 * \param blender_id: id of the blender between 0 and EVIEWITF_MAX_BLENDER
 * \param attributes: pointer on the structure to be filled

 * \return state of the function. Return 0 if okay
 */
int eviewitf_blender_get_attributes(int blender_id, eviewitf_device_attributes_t *attributes) {
    int ret = EVIEWITF_OK;
    /* Get the blenders attributes */
    struct eviewitf_mfis_blending_attributes *blender_attributes =
        eviewitf_get_blender_attributes(blender_id);

    /* Test blender id */
    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
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
        attributes->buffer_size = blender_attributes->buffer_size;
        attributes->width = blender_attributes->width;
        attributes->height = blender_attributes->height;
        attributes->dt = blender_attributes->dt;
    }

    return ret;
}

/**
 * \fn eviewitf_blender_write_frame
 * \brief Write a frame to a blender

 * \param in blender_id: id of the camera
 * \param in buffer_size: size of the virtual camera buffer
 * \param in buffer: virtual camera buffer
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_blender_write_frame(int blender_id, uint8_t* frame_buffer, uint32_t buffer_size) {
    int ret = EVIEWITF_OK;

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
        // Test blender has been opened
        if (file_blenders[blender_id] == -1) {
            ret = EVIEWITF_NOT_OPENED;
        }
    }

    if (ret >= EVIEWITF_OK) {
        /* Write the frame in the virtual camera */
        if (write(file_blenders[blender_id], frame_buffer, buffer_size) < 0) {
            ret = EVIEWITF_FAIL;
        }
    }

    return ret;
}

/**
 * \file eviewitf_streamer.c
 * \brief Communication API between A53 and R7 CPUs for streamer devices
 * \author eSoftThings
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include "eviewitf_priv.h"

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

/******************************************************************************************
 * Functions
 ******************************************************************************************/

int streamer_open(int device_id) {
    char device_name[DEVICE_CAMERA_MAX_LENGTH];

    /* Get mfis device filename */
    snprintf(device_name, DEVICE_CAMERA_MAX_LENGTH, DEVICE_CAMERA_NAME, device_id);
    return open(device_name, O_WRONLY);
}

int streamer_close(int file_descriptor) { return close(file_descriptor); }

int streamer_write(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size) {
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
    /* Test streamer id */
    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    /* Open device */
    return device_open(streamer_id + EVIEWITF_OFFSET_STREAMER);
}

/**
 * \fn int eviewitf_streamer_close(int streamer_id)
 * \brief Close a streamer device
 *
 * \param streamer_id: id of the streamer between 0 and EVIEWITF_MAX_STREAMER

 * \return state of the function. Return 0 if okay
 */
int eviewitf_streamer_close(int streamer_id) {
    /* Test streamer id */
    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_close(streamer_id + EVIEWITF_OFFSET_STREAMER);
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
    /* Test streamer id */
    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_get_attributes(streamer_id + EVIEWITF_OFFSET_STREAMER, attributes);
}

/**
 * \fn eviewitf_streamer_write_frame
 * \brief Write a frame to a streamer

 * \param in streamer_id: id of the streamer
 * \param in buffer_size: size of the streamer buffer
 * \param in buffer: streamer buffer
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_streamer_write_frame(int streamer_id, uint8_t *frame_buffer, uint32_t buffer_size) {
    /* Test streamer id */
    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_write(streamer_id + EVIEWITF_OFFSET_STREAMER, frame_buffer, buffer_size);
}
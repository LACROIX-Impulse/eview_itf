/**
 * \file
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

/**
 * \fn int streamer_open(int device_id)
 * \brief open a streamer device
 *
 * \param device_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 *
 * \return file descriptor or -1
 */
int streamer_open(int device_id) {
    char device_name[DEVICE_CAMERA_MAX_LENGTH];

    /* Get mfis device filename */
    snprintf(device_name, DEVICE_CAMERA_MAX_LENGTH, DEVICE_CAMERA_NAME, device_id);
    return open(device_name, O_WRONLY);
}

/**
 * \fn int eviewitf_streamer_open(int streamer_id)
 * \brief Open a streamer device
 * \ingroup streamer
 *
 * \param[in] streamer_id id of the streamer between 0 and EVIEWITF_MAX_STREAMER
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A streamer must be opened before to be able to use it (write_frame). A streamer should not be opened by two different
 * process at the same time.
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
 * \ingroup streamer
 *
 * \param[in] streamer_id id of the streamer between 0 and EVIEWITF_MAX_STREAMER
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A streamer should be closed before to stop the process that opened it.
 */
int eviewitf_streamer_close(int streamer_id) {
    /* Test streamer id */
    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_close(streamer_id + EVIEWITF_OFFSET_STREAMER);
}

/**
 * \fn int eviewitf_streamer_get_attributes(int streamer_id, eviewitf_device_attributes_t* attributes)
 * \brief Get the attributes of a streamer such as buffer size
 * \ingroup streamer
 *
 * \param[in] streamer_id id of the streamer between 0 and EVIEWITF_MAX_STREAMER
 * \param[out] attributes pointer on the structure to be filled
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The attributes that can be retrieved through this function are the ones defined in the structure
 * eviewitf_device_attributes_t.
 */
int eviewitf_streamer_get_attributes(int streamer_id, eviewitf_device_attributes_t *attributes) {
    /* Test streamer id */
    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_get_attributes(streamer_id + EVIEWITF_OFFSET_STREAMER, attributes);
}

/**
 * \fn eviewitf_streamer_write_frame(int streamer_id, uint8_t* frame_buffer, uint32_t buffer_size)
 * \brief Write a frame to a streamer
 * \ingroup streamer
 *
 * \param[in] streamer_id id of the camera
 * \param[in] buffer_size size of the streamer buffer
 * \param[in] frame_buffer streamer buffer
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A streamer can be selected for being displayed on the screen connected to the eCube through
 * eviewitf_display_select_streamer. buffer_size should be equal to the size of frame_buffer and can be retrieved
 * through a call to eviewitf_streamer_get_attributes.
 */
int eviewitf_streamer_write_frame(int streamer_id, uint8_t *frame_buffer, uint32_t buffer_size) {
    /* Test streamer id */
    if ((streamer_id < 0) || (streamer_id >= EVIEWITF_MAX_STREAMER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_write(streamer_id + EVIEWITF_OFFSET_STREAMER, frame_buffer, buffer_size);
}
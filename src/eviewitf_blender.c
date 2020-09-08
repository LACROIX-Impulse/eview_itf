/**
 * \file
 * \brief Communication API between A53 and R7 CPUs for blender devices
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
 * \fn int blender_open(int device_id)
 * \brief open a blender device
 *
 * \param device_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 *
 * \return file descriptor or -1
 */
int blender_open(int device_id) {
    char device_name[DEVICE_BLENDER_MAX_LENGTH];

    /* Get mfis device filename */
    snprintf(device_name, DEVICE_BLENDER_MAX_LENGTH, DEVICE_BLENDER_NAME,
             device_id - EVIEWITF_OFFSET_BLENDER + 2); /* O2 and O3 */
    return open(device_name, O_WRONLY);
}

/**
 * \fn int eviewitf_blender_open(int blender_id)
 * \brief Open a blender device
 * \ingroup blender
 *
 * \param[in] blender_id id of the blender between 0 and EVIEWITF_MAX_BLENDER
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A blender must be opened before to be able to use it (write_frame). A blender should not be opened by two different
 * process at the same time.
 */
int eviewitf_blender_open(int blender_id) {
    /* Test blender id */
    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    /* Open device */
    return device_open(blender_id + EVIEWITF_OFFSET_BLENDER);
}

/**
 * \fn int eviewitf_blender_close(int blender_id)
 * \brief Close a blender device
 * \ingroup blender
 *
 * \param[in] blender_id id of the blender between 0 and EVIEWITF_MAX_BLENDER
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A blender should be closed before to stop the process that opened it.
 */
int eviewitf_blender_close(int blender_id) {
    /* Test blender id */
    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_close(blender_id + EVIEWITF_OFFSET_BLENDER);
}

/**
 * \fn int eviewitf_blender_get_attributes(int blender_id, eviewitf_device_attributes_t* attributes)
 * \brief Get the attributes of a blender such as buffer size
 * \ingroup blender
 *
 * \param[in] blender_id id of the blender between 0 and EVIEWITF_MAX_BLENDER
 * \param[out] attributes pointer on the structure to be filled
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The attributes that can be retrieved through this function are the ones defined in the structure
 * eviewitf_device_attributes_t.
 */
int eviewitf_blender_get_attributes(int blender_id, eviewitf_device_attributes_t *attributes) {
    /* Test blender id */
    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_get_attributes(blender_id + EVIEWITF_OFFSET_BLENDER, attributes);
}

/**
 * \fn eviewitf_blender_write_frame(int blender_id, uint8_t* frame_buffer, uint32_t buffer_size)
 * \brief Write a frame to a blender
 * \ingroup blender
 *
 * \param[in] blender_id: id of the blender
 * \param[in] buffer_size: size of the blender frame buffer
 * \param[in] frame_buffer: blender frame buffer
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A blender can be selected for being displayed, over the currently selected camera or streamer, on the screen
 * connected to the eCube through eviewitf_display_select_blender.
 */
int eviewitf_blender_write_frame(int blender_id, uint8_t *frame_buffer, uint32_t buffer_size) {
    /* Test blender id */
    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_write(blender_id + EVIEWITF_OFFSET_BLENDER, frame_buffer, buffer_size);
}

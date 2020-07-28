/**
 * \file eviewitf_blender.c
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

int blender_open(int device_id) {
    char device_name[DEVICE_BLENDER_MAX_LENGTH];

    /* Get mfis device filename */
    snprintf(device_name, DEVICE_BLENDER_MAX_LENGTH, DEVICE_BLENDER_NAME,
             device_id - EVIEWITF_OFFSET_BLENDER + 2); /* O2 and O3 */
    printf("blender_open device_name %s\n", device_name);
    return open(device_name, O_WRONLY);
}

int blender_close(int file_descriptor) { return close(file_descriptor); }

int blender_write(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size) {
    return write(file_descriptor, frame_buffer, buffer_size);
}

/**
 * \fn int eviewitf_blender_open(int blender_id)
 * \brief Open a blender device
 *
 * \param blender_id: id of the blender between 0 and EVIEWITF_MAX_BLENDER

 * \return state of the function. Return 0 if okay
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
 *
 * \param blender_id: id of the blender between 0 and EVIEWITF_MAX_BLENDER

 * \return state of the function. Return 0 if okay
 */
int eviewitf_blender_close(int blender_id) {
    /* Test blender id */
    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_close(blender_id + EVIEWITF_OFFSET_BLENDER);
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
    /* Test blender id */
    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_get_attributes(blender_id + EVIEWITF_OFFSET_BLENDER, attributes);
}

/**
 * \fn eviewitf_blender_write_frame
 * \brief Write a frame to a blender

 * \param in blender_id: id of the blender
 * \param in buffer_size: size of the blender frame buffer
 * \param in buffer: blender frame buffer
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_blender_write_frame(int blender_id, uint8_t *frame_buffer, uint32_t buffer_size) {
    /* Test blender id */
    if ((blender_id < 0) || (blender_id >= EVIEWITF_MAX_BLENDER)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_write(blender_id + EVIEWITF_OFFSET_BLENDER, frame_buffer, buffer_size);
}

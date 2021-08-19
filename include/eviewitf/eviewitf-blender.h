/**
 * \file
 * \brief Header for eViewItf API regarding blenders
 * \author LACROIX Impulse
 * \copyright Copyright (c) 2019-2021 LACROIX Impulse
 * \ingroup blender
 *
 * Communication API between A53 and R7 CPUs for blender devices
 *
 * \addtogroup blender
 * \{
 */

#ifndef EVIEWITF_BLENDER_H
#define EVIEWITF_BLENDER_H

#include <stdint.h>
#include "eviewitf-structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def EVIEWITF_MAX_BLENDER
 * \brief Max number of blender devices
 */
#define EVIEWITF_MAX_BLENDER 2

/**
 * \fn int eviewitf_blender_open(int blender_id)
 * \brief Open a blender device
 *
 * \param[in] blender_id id of the blender between 0 and EVIEWITF_MAX_BLENDER
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A blender must be opened before to be able to use it (write_frame). A blender should not be opened by two different
 * process at the same time.
 */
int eviewitf_blender_open(int blender_id);

/**
 * \fn int eviewitf_blender_close(int blender_id)
 * \brief Close a blender device
 *
 * \param[in] blender_id id of the blender between 0 and EVIEWITF_MAX_BLENDER
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A blender should be closed before to stop the process that opened it.
 */
int eviewitf_blender_close(int blender_id);

/**
 * \fn int eviewitf_blender_get_attributes(int blender_id, eviewitf_device_attributes_t* attributes)
 * \brief Get the attributes of a blender such as buffer size
 *
 * \param[in] blender_id id of the blender between 0 and EVIEWITF_MAX_BLENDER
 * \param[out] attributes pointer on the structure to be filled
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The attributes that can be retrieved through this function are the ones defined in the structure
 * eviewitf_device_attributes_t.
 */
int eviewitf_blender_get_attributes(int blender_id, eviewitf_device_attributes_t* attributes);

/**
 * \fn eviewitf_blender_write_frame(int blender_id, uint8_t* frame_buffer, uint32_t buffer_size)
 * \brief Write a frame to a blender
 *
 * \param[in] blender_id: id of the blender
 * \param[in] buffer_size: size of the blender frame buffer
 * \param[in] frame_buffer: blender frame buffer
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A blender can be selected for being displayed, over the currently selected camera or streamer, on the screen
 * connected to the eCube through eviewitf_display_select_blender.
 */
int eviewitf_blender_write_frame(int blender_id, uint8_t* frame_buffer, uint32_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_BLENDER_H */

/*! \} */

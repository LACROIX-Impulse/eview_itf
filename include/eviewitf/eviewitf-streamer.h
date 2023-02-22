/**
 * \file
 * \brief Header for eViewItf API regarding streamers
 * \author LACROIX Impulse
 * \copyright Copyright (c) 2019-2022 LACROIX Impulse
 * \ingroup streamer
 *
 * Communication API between A53 and R7 CPUs for streamer devices
 *
 * \addtogroup streamer
 * \{
 */

#ifndef EVIEWITF_STREAMER_H
#define EVIEWITF_STREAMER_H

#include <stdint.h>
#include "eviewitf-structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def EVIEWITF_MAX_STREAMER
 * \brief Max number of streamer devices
 */
#define EVIEWITF_MAX_STREAMER 8

/**
 * \fn eviewitf_ret_t eviewitf_streamer_open(int streamer_id)
 * \brief Open a streamer device
 *
 * \param[in] streamer_id id of the streamer between 0 and EVIEWITF_MAX_STREAMER
 * \return return code as specified by the eviewitf_ret_t enumeration.
 *
 * A streamer must be opened before to be able to use it (write_frame). A streamer should not be opened by two different
 * process at the same time.
 */
eviewitf_ret_t eviewitf_streamer_open(int streamer_id);

/**
 * \fn eviewitf_ret_t eviewitf_streamer_close(int streamer_id)
 * \brief Close a streamer device
 *
 * \param[in] streamer_id id of the streamer between 0 and EVIEWITF_MAX_STREAMER
 * \return return code as specified by the eviewitf_ret_t enumeration.
 *
 * A streamer should be closed before to stop the process that opened it.
 */
eviewitf_ret_t eviewitf_streamer_close(int streamer_id);

/**
 * \fn eviewitf_ret_t eviewitf_streamer_get_attributes(int streamer_id, eviewitf_device_attributes_t* attributes)
 * \brief Get the attributes of a streamer such as buffer size
 *
 * \param[in] streamer_id id of the streamer between 0 and EVIEWITF_MAX_STREAMER
 * \param[out] attributes pointer on the structure to be filled
 * \return return code as specified by the eviewitf_ret_t enumeration.
 *
 * The attributes that can be retrieved through this function are the ones defined in the structure
 * eviewitf_device_attributes_t.
 */
eviewitf_ret_t eviewitf_streamer_get_attributes(int streamer_id, eviewitf_device_attributes_t* attributes);

/**
 * \fn eviewitf_streamer_write_frame(int streamer_id, uint8_t* frame_buffer, uint32_t buffer_size)
 * \brief Write a frame to a streamer
 *
 * \param[in] streamer_id id of the camera
 * \param[in] buffer_size size of the streamer buffer
 * \param[in] frame_buffer streamer buffer
 * \return return code as specified by the eviewitf_ret_t enumeration.
 *
 * A streamer can be selected for being displayed on the screen connected to the eCube through
 * eviewitf_display_select_streamer. buffer_size should be equal to the size of frame_buffer and can be retrieved
 * through a call to eviewitf_streamer_get_attributes.
 */
eviewitf_ret_t eviewitf_streamer_write_frame(int streamer_id, uint8_t* frame_buffer, uint32_t buffer_size);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_H */

/*! \} */

/**
 * \file
 * \brief Header for eViewItf API
 * \author eSoftThings
 * \copyright Copyright (c) 2015-2020 eSoftThings. All rights reserved.
 *
 * Communication API between A53 and R7 CPUs
 */

#ifndef EVIEWITF_H
#define EVIEWITF_H

#include <stdint.h>

#include "eviewitf/eviewitf-structs.h"
#include "eviewitf/eviewitf-camera.h"
#include "eviewitf/eviewitf-streamer.h"
#include "eviewitf/eviewitf-blender.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def EVIEWITF_MONITORING_INFO_SIZE
 * \brief Size of the monitoring uint32_t table
 */
#define EVIEWITF_MONITORING_INFO_SIZE 6

/**
 * \fn eviewitf_init
 * \brief Initialize the eViewItf API
 * \ingroup eview
 *
 * \return Return code as specified by the eviewitf_return_code enumeration.
 *
 * Initialize the eViewItf API by opening a communication with eView and by retrieving devices information from eView.
 * This function must be called before any other function of this API.
 * Otherwise, the other functions will return the error code EVIEWITF_NOT_INITIALIZED (eviewitf_return_state).
 */
int eviewitf_init(void);

/**
 * \fn eviewitf_deinit
 * \brief De-initialize the eViewItf API
 * \ingroup eview
 *
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * De-initialize the eViewItf API by closing the communication with eView.
 */
int eviewitf_deinit(void);

/**
 * \fn eviewitf_set_R7_heartbeat_mode(uint32_t mode)
 * \brief Activate or deactivate eView heartbeat.
 * \ingroup eview
 *
 * \param[in] mode 0 to deactivate heartbeat other to activate it
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The eView heartbeat can be activated to check if eView is still running as it should.
 * With the heartbeat activated, eView will regularly send a message over the eCube’s USB Debug port.
 * This is a debugging function. This function should not be used in a normal behavior.
 * However, it can help to identify the cause of an EVIEWITF_BLOCKED (eviewitf_return_code) error code.
 */
int eviewitf_set_R7_heartbeat_mode(uint32_t mode);

/**
 * \fn eviewitf_set_R7_boot_mode(uint32_t mode)
 * \brief Set a specific boot mode to eView.
 * \ingroup eview
 *
 * \param[in] mode requets a specific R7 boot mode
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * An eView specific mode can be set under peculiar conditions.
 * This function is not needed most of the time. It can be used to tune the eView’s behavior for some customers’
 * requests.
 */
int eviewitf_set_R7_boot_mode(uint32_t mode);

/**
 * \fn eviewitf_get_eview_version
 * \brief Retrieve eView version
 * \ingroup version
 *
 * \return returns a pointer on a string containing the eView version number.
 *
 * Retrieve the running eView version.
 */
const char* eviewitf_get_eview_version(void);

/**
 * \fn eviewitf_get_eviewitf_version
 * \brief Get the version of eViewItf.
 * \ingroup version
 *
 * \return returns a pointer on a string containing the eViewItf version number.
 */
const char* eviewitf_get_eviewitf_version(void);

/**
 * \fn eviewitf_get_monitoring_info(uint32_t* data, uint8_t size)
 * \brief Request R7 to get monitoring info.
 * \ingroup eview
 *
 * \param[out] data pointer where to store monitoring info
 * \param[in] size size of the data table, should not be greater than EVIEWITF_MONITORING_INFO_SIZE
 * \return state of the function. Return 0 if okay
 *
 * Content is voluntary not explicitly described in this interface, can be project specific.
 */
int eviewitf_get_monitoring_info(uint32_t* data, uint8_t size);

/**
 * \fn eviewitf_get_R7_boot_mode(uint32_t *mode)
 * \brief Get current eView boot mode.
 * \ingroup eview
 *
 * \param[out] mode current/active boot mode of eView component
 * \return state of the function. Return 0 if okay
 *
 * An eView specific mode can be set under peculiar conditions.
 * This function is not needed most of the time. It can be used to tune the eView’s behavior for some customers’
 * requests.
 */
int eviewitf_get_R7_boot_mode(uint32_t* mode);

/**
 * \fn eviewitf_display_select_camera(int cam_id)
 * \brief Select a camera input to be displayed on the screen connected to the eCube
 * \ingroup display
 *
 * \param[in] cam_id: id of the camera
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * Replace the currently displayed camera or streamer.
 */
int eviewitf_display_select_camera(int cam_id);

/**
 * \fn eviewitf_display_select_streamer(int streamer_id)
 * \brief Select a streamer to be printed on the screen connected to the eCube
 * \ingroup display
 *
 * \param[in] streamer_id: id of the streamer
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * Replace the currently displayed camera or streamer.
 */
int eviewitf_display_select_streamer(int streamer_id);

/**
 * \fn eviewitf_display_select_blender(int blender_id)
 * \brief Select a blender to be displayed, over the currently selected camera or streamer, on the screen connected to
 * the eCube.
 * \ingroup display
 *
 * \param[in] blender_id: id of the blender
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * Calling this function with blender_id not included between 0 and EVIEWITF_MAX_BLENDER – 1 (API macros) deactivates
 * the blender (no more overlay on the currently displayed camera or streamer).
 */
int eviewitf_display_select_blender(int blender_id);

/**
 * \fn eviewitf_display_select_cropping(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2)
 * \brief Crop a ROI in the current display.
 * \ingroup display
 *
 * \param[in] x1: set first coordinate X position
 * \param[in] y1: set first coordinate Y position
 * \param[in] x2: set second coordinate X position
 * \param[in] y2: set second coordinate Y position
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * Setting all the coordinates to 0 deactivates the cropping.
 */
int eviewitf_display_select_cropping(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_H */

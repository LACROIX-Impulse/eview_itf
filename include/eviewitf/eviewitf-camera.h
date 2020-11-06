/**
 * \file
 * \brief Header for eViewItf API regarding cameras
 * \author eSoftThings
 * \copyright Copyright (c) 2015-2020 eSoftThings. All rights reserved.
 * \ingroup camera
 *
 * Communication API between A53 and R7 CPUs for camera devices
 * 
 * \addtogroup camera
 * \{
 */

#ifndef EVIEWITF_CAMERA_H
#define EVIEWITF_CAMERA_H

#include <stdint.h>
#include "eviewitf-structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def EVIEWITF_MAX_CAMERA
 * \brief Max number of camera devices
 */
#define EVIEWITF_MAX_CAMERA 8

/**
 * \fn int eviewitf_camera_open(int cam_id)
 * \brief Open a camera device
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A camera must be opened before to be able to use it (get_frame, poll, get_parameter, set_parameter).
 * A camera should not be opened by two different process at the same time.
 */
int eviewitf_camera_open(int cam_id);

/**
 * \fn int eviewitf_camera_close(int cam_id)
 * \brief Close a camera device
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A camera should be closed before to stop the process that opened it.
 */
int eviewitf_camera_close(int cam_id);

/**
 * \fn int eviewitf_camera_get_attributes(int cam_id, eviewitf_device_attributes_t* attributes)
 * \brief Get the attributes of a camera such as buffer size
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] attributes pointer on the structure to be filled
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The attributes that can be retrieved through this function are the ones defined in the structure
 * eviewitf_device_attributes_t.
 */
int eviewitf_camera_get_attributes(int cam_id, eviewitf_device_attributes_t* attributes);

/**
 * \fn int eviewitf_camera_get_frame(int cam_id, uint8_t *frame_buffer, uint32_t buffer_size)
 * \brief Get a copy (from eView context memory) of the latest frame received from a camera.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] frame_buffer buffer to store the incoming frame
 * \param[in] buffer_size buffer size for coherency check
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The frame_buffer must be allocated by the customer application before to call this function.
 * The size to be allocated must be the same than specified by buffer_size and can be retrieved through a call to
 * eviewitf_camera_get_attributes..
 */
int eviewitf_camera_get_frame(int cam_id, uint8_t* frame_buffer, uint32_t buffer_size);

/**
 * \fn int eviewitf_camera_extract_metadata(uint8_t *buf, uint32_t buffer_size,
                              eviewitf_frame_metadata_info_t *frame_metadata)
 * \brief Extract metadata from a frame buffer
 *
 * \param[in] buf pointer on the buffer where the frame is stored
 * \param[in] buffer_size size of the buffer
 * \param[out] frame_metadata pointer on metadata structure to be filled
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The metadata that can be retrieved by this functions are the ones defined in the structure
 eviewitf_frame_metadata_info_t.
 */
int eviewitf_camera_extract_metadata(uint8_t* buf, uint32_t buffer_size,
                                     eviewitf_frame_metadata_info_t* frame_metadata);
/**
 * \fn int eviewitf_camera_poll(int* cam_id, int nb_cam, int ms_timeout, short* event_return)
 * \brief Poll on multiple cameras to check a new frame is available
 *
 * \param[in] cam_id table of camera ids to poll on (id between 0 and EVIEWITF_MAX_CAMERA)
 * \param[in] nb_cam number of cameras on which the polling applies
 * \param[in] ms_timeout dealy the function should block waiting for a frame, negative value means infinite
 * \param[out] event_return detected events for each camera, 0 if no frame, 1 if a frame is available
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The function eviewitf_camera_get_frame gives a copy of the latest frame received from a camera.
 * However, it is not a blocking function. Therefore, it does not allow to wait for a new frame to become available.
 * This waiting is possible through this eviewitf_camera_poll function. Instead of having one function per camera, the
 * poll can be done on several cameras. Thanks to this function, a process can wait for a new frame to become available
 * among a list of cameras. As soon as one camera of the list will get a new frame, the poll will return.
 */
int eviewitf_camera_poll(int* cam_id, int nb_cam, int ms_timeout, short* event_return);

/**
 * \fn eviewitf_camera_get_parameter(int cam_id, uint32_t reg_address, uint32_t* reg_value)
 * \brief Get a parameter of a camera.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] reg_address Register address
 * \param[out] reg_value Register Value
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_parameter(int cam_id, uint32_t reg_address, uint32_t* reg_value);

/**
 * \fn eviewitf_camera_set_parameter(int cam_id, uint32_t reg_address, uint32_t reg_value)
 * \brief Set a parameter of a camera.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] reg_address Register address
 * \param[in] reg_value Register Value to set
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_parameter(int cam_id, uint32_t reg_address, uint32_t reg_value);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_CAMERA_H */

/*! \} */

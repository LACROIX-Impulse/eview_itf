/**
 * \file
 * \brief Header for eViewItf API regarding cameras
 * \author LACROIX Impulse
 * \copyright Copyright (c) 2019-2021 LACROIX Impulse
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
 * \brief Possible camera test patterns
 * @{
 */
#define EVIEWITF_TEST_PATTERN_NONE             (0)   /* No test pattern */
#define EVIEWITF_TEST_PATTERN_SOLID_RED        (1)   /* Solid color - red */
#define EVIEWITF_TEST_PATTERN_SOLID_GREEN      (2)   /* Solid color - green */
#define EVIEWITF_TEST_PATTERN_SOLID_BLUE       (3)   /* Solid color - blue */
#define EVIEWITF_TEST_PATTERN_SOLID_VBAR       (4)   /* Vertical bars */
#define EVIEWITF_TEST_PATTERN_SOLID_VBAR_FADED (5)   /* Vertical bars faded */
#define EVIEWITF_TEST_PATTERN_CUSTOM0          (16)  /* Custom pattern */
#define EVIEWITF_TEST_PATTERN_CUSTOM1          (17)  /* Custom pattern */
#define EVIEWITF_TEST_PATTERN_CUSTOM2          (18)  /* Custom pattern */
#define EVIEWITF_TEST_PATTERN_CUSTOM3          (19)  /* Custom pattern */
#define EVIEWITF_TEST_PATTERN_CUSTOM4          (20)  /* Custom pattern */
#define EVIEWITF_TEST_PATTERN_UNKNOWN          (255) /* Unknown test pattern */
/** @} */

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
 * \fn int eviewitf_camera_start(int cam_id)
 * \brief Request to start a camera
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_start(int cam_id);

/**
 * \fn int eviewitf_camera_stop(int cam_id)
 * \brief Request to stop a camera
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_stop(int cam_id);

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
 * \fn int eviewitf_camera_get_frame_segment(int cam_id, uint8_t* buffer, uint32_t size, uint32_t offset)
 * \brief Get a copy (from eView context memory) of a segment of the latest frame received from a
 * camera. Segment from offset to offset + buffer_size
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] buffer buffer to store the incoming segment
 * \param[in] size buffer size of the segment
 * \param[in] offset offset of the segment from frame buffer start adress
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The buffer must be allocated by the customer application before to call this function.
 * The size to be allocated for a particular segement can be retrieved from frame metadata and can retrieved through a
 * call to eviewitf_camera_get_frame_metadata.
 */
int eviewitf_camera_get_frame_segment(int cam_id, uint8_t* buffer, uint32_t size, uint32_t offset);

/**
 * \fn int eviewitf_camera_get_frame_metadata(int cam_id, eviewitf_frame_metadata_info_t* frame_metadata)
 * \brief Read frame metadata (which is a frame segment)
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] frame_metadata pointer on metadata structure to be filled
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The metadata that can be retrieved by this functions are the ones defined in the structure
 eviewitf_frame_metadata_info_t.
 */
int eviewitf_camera_get_frame_metadata(int cam_id, eviewitf_frame_metadata_info_t* frame_metadata);

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

/**
 * \fn eviewitf_camera_get_exposure(int cam_id, uint32_t *exposure_us, uint32_t *gain_thou)
 * \brief Get camera's exposure time and gain.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] exposure_us pointer to the returned exposure time in micro seconds
 * \param[out] gain_thou pointer to the returned gain in 1/1000 of unit
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_exposure(int cam_id, uint32_t* exposure_us, uint32_t* gain_thou);

/**
 * \fn eviewitf_camera_get_digital_gains(int cam_id, uint16_t *dg_cf00, uint16_t *dg_cf01, uint16_t *dg_cf10, uint16_t
 * *dg_cf11) \brief Get camera's CFA patterns digital gains.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] dg_cf00 pointer to the returned CFA 00 digital gain
 * \param[out] dg_cf01 pointer to the returned CFA 01 digital gain
 * \param[out] dg_cf10 pointer to the returned CFA 10 digital gain
 * \param[out] dg_cf11 pointer to the returned CFA 11 digital gain
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_digital_gains(int cam_id, uint16_t* dg_cf00, uint16_t* dg_cf01, uint16_t* dg_cf10,
                                      uint16_t* dg_cf11);

/**
 * \fn eviewitf_camera_get_frame_rate(int cam_id, uint16_t *fps)
 * \brief Get camera's frame rate.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] fps pointer to the returned camera frame rate
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_frame_rate(int cam_id, uint16_t* fps);

/**
 * \fn eviewitf_camera_set_frame_rate(int cam_id, uint16_t fps)
 * \brief Set camera's frame rate.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] fps camera frame rate
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_frame_rate(int cam_id, uint16_t fps);

/**
 * \fn eviewitf_camera_set_digital_gains(int cam_id, uint16_t dg_cf00, uint16_t dg_cf01, uint16_t dg_cf10, uint16_t
 * dg_cf11) \brief Set camera's CFA patterns digital gains.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] dg_cf00 CFA 00 digital gain
 * \param[out] dg_cf01 CFA 01 digital gain
 * \param[out] dg_cf10 CFA 10 digital gain
 * \param[out] dg_cf11 CFA 11 digital gain
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_digital_gains(int cam_id, uint16_t dg_cf00, uint16_t dg_cf01, uint16_t dg_cf10,
                                      uint16_t dg_cf11);

/**
 * \fn eviewitf_camera_get_min_exposure(int cam_id, uint32_t *exposure_us, uint32_t *gain_thou)
 * \brief Get camera's minimum exposure time and gain.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] exposure_us pointer to the returned exposure time in micro seconds
 * \param[out] gain_thou pointer to the returned gain in 1/1000 of unit
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_min_exposure(int cam_id, uint32_t* exposure_us, uint32_t* gain_thou);

/**
 * \fn eviewitf_camera_get_max_exposure(int cam_id, uint32_t *exposure_us, uint32_t *gain_thou)
 * \brief Get camera's maximum exposure time and gain.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] exposure_us pointer to the returned exposure time in micro seconds
 * \param[out] gain_thou pointer to the returned gain in 1/1000 of unit
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_max_exposure(int cam_id, uint32_t* exposure_us, uint32_t* gain_thou);

/**
 * \fn  eviewitf_camera_set_exposure(int cam_id, uint32_t exposure_us, uint32_t gain_thou)
 * \brief Set camera's exposure time.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] exposure_us exposure time in micro seconds
 * \param[out] gain_thou gain in 1/1000 of unit
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_exposure(int cam_id, uint32_t exposure_us, uint32_t gain_thou);

/**
 * \fn eviewitf_camera_get_frame_offset(int cam_id, uint32_t *x_offset, uint32_t *y_offset)
 * \brief Get camera's frame offset relative to camera sensor
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] x_offset pointer to the returned frame offset (width)
 * \param[out] y_offset pointer to the returned frame offset (height)
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_frame_offset(int cam_id, uint32_t* x_offset, uint32_t* y_offset);

/**
 * \fn eviewitf_camera_set_frame_offset(int cam_id, uint32_t x_offset, uint32_t y_offset)
 * \brief Set camera's frame offset relative to camera sensor
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] x_offset frame offset (width)
 * \param[in] y_offset frame offset (height)
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_frame_offset(int cam_id, uint32_t x_offset, uint32_t y_offset);

/**
 * \fn eviewitf_camera_get_test_pattern(int cam_id, uint8_t *pattern)
 * \brief Get camera's test pattern
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] pattern the current test pattern used
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_test_pattern(int cam_id, uint8_t* pattern);

/**
 * \fn int eviewitf_camera_set_test_pattern(int cam_id, uint8_t pattern)
 * \brief Set camera's test pattern
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] pattern test pattern used
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_test_pattern(int cam_id, uint8_t pattern);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_CAMERA_H */

/*! \} */

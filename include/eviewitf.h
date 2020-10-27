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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \def EVIEWITF_MAX_CAMERA
 * \brief Max number of camera devices
 */
#define EVIEWITF_MAX_CAMERA 8

/**
 * \def EVIEWITF_MAX_STREAMER
 * \brief Max number of streamer devices
 */
#define EVIEWITF_MAX_STREAMER 8

/**
 * \def EVIEWITF_MAX_BLENDER
 * \brief Max number of blender devices
 */
#define EVIEWITF_MAX_BLENDER 2
/**
 * \def EVIEWITF_MONITORING_INFO_SIZE
 * \brief Size of the monitoring uint32_t table
 */
#define EVIEWITF_MONITORING_INFO_SIZE 6

/**
 * \enum eviewitf_return_code
 * \brief Return codes used by eViewItf API
 *        A negative value is therefore an error
 */
typedef enum {
    EVIEWITF_OK,                   /*!< No issues */
    EVIEWITF_BLOCKED = -1,         /*!< eViewItf is blocked in a process / wait for an eView answer */
    EVIEWITF_INVALID_PARAM = -2,   /*!< Bad parameters have been set in a function call */
    EVIEWITF_NOT_INITIALIZED = -3, /*!< The API is not initialized before a function call */
    EVIEWITF_NOT_OPENED = -4,      /*!< The targeted device is not opened */
    EVIEWITF_FAIL = -5,            /*!< Something has failed during the function call */
} eviewitf_return_code;

/**
 * \struct eviewitf_frame_metadata_info_t
 * \brief Pointers to current camera frame metadata
 *
 * The frames metadata are extra information that can be found at the end of a frame.
 * Note: The frame synchronization flag can be used in the customer application to get a synchronization point between
 * two input cameras. For instance, it can be used to synchronize a first camera with a second camera which has a slower
 * framerate. The frame size can be used to verify the data integrity.
 */
typedef struct {
    uint32_t frame_width;         /*!< The frame width (in pixels) */
    uint32_t frame_height;        /*!< The frame height (in pixels) */
    uint32_t frame_bpp;           /*!< The number of bytes per pixels */
    uint32_t frame_timestamp_lsb; /*!< The timestamp (LSB). */
    uint32_t frame_timestamp_msb; /*!< The timestamp (MSB). */
    uint32_t frame_sync;          /*!< A frame synchronization flag */
    uint32_t reserved[24];        /*!< 32 metadata fields in total. */
    uint32_t frame_size;          /*!< The frame size (in bytes) */
    uint32_t magic_number;        /*!< A memory pattern which marks the end of the metadata (magic number) */
} eviewitf_frame_metadata_info_t;

/**
 * \struct eviewitf_device_attributes_t
 * \brief Structure to get a device (camera, streamer or blender) attributes
 *
 * The device attributes gather some information on a device. The device can be a camera, a streamer or a blender.
 * Note: The data type possible values are conformed to the “MIPI Alliance Specification for CSI-2”.
 * Extra data types can also be added depending on customers’ requests.
 *
 */
typedef struct {
    uint32_t buffer_size; /*!< The buffer size (reception buffer for a camera / writing buffer for a streamer or a
                             blender) */
    uint32_t width;       /*!< The frame width (in pixels) */
    uint32_t height;      /*!< The frame height (in pixels) */
    uint16_t dt;          /*!< The data type (Y only, YUV, RGB…) */
} eviewitf_device_attributes_t;

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
 * \fn int eviewitf_camera_open(int cam_id)
 * \brief Open a camera device
 * \ingroup cameras
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
 * \ingroup cameras
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
 * \ingroup cameras
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
 * \ingroup cameras
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
 * \ingroup cameras
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
 * \ingroup cameras
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
 * \ingroup cameras
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
 * \ingroup cameras
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] reg_address Register address
 * \param[in] reg_value Register Value to set
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_parameter(int cam_id, uint32_t reg_address, uint32_t reg_value);

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
int eviewitf_streamer_open(int streamer_id);

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
int eviewitf_streamer_close(int streamer_id);

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
int eviewitf_streamer_get_attributes(int streamer_id, eviewitf_device_attributes_t* attributes);

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
int eviewitf_streamer_write_frame(int streamer_id, uint8_t* frame_buffer, uint32_t buffer_size);

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
int eviewitf_blender_open(int blender_id);

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
int eviewitf_blender_close(int blender_id);

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
int eviewitf_blender_get_attributes(int blender_id, eviewitf_device_attributes_t* attributes);

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
int eviewitf_blender_write_frame(int blender_id, uint8_t* frame_buffer, uint32_t buffer_size);

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

/**
 * @file eviewitf-priv.h
 * @brief Internal header for private structures
 * @author LACROIX Impulse
 */

#ifndef EVIEWITF_PRIV_H
#define EVIEWITF_PRIV_H

#include <stdint.h>

#include "eviewitf.h"

/******************************************************************************************
 * Private Definitions
 ******************************************************************************************/

/**
 * @brief eViewItf maximum number of device
 */
#define EVIEWITF_MAX_DEVICES (EVIEWITF_MAX_CAMERA + EVIEWITF_MAX_STREAMER + EVIEWITF_MAX_BLENDER)

/**
 * @brief eViewItf camera offset
 */
#define EVIEWITF_OFFSET_CAMERA (0)

/**
 * @brief eViewItf streamer offset
 */
#define EVIEWITF_OFFSET_STREAMER (EVIEWITF_MAX_CAMERA)

/**
 * @brief eViewItf blender offset
 */
#define EVIEWITF_OFFSET_BLENDER (EVIEWITF_OFFSET_STREAMER + EVIEWITF_MAX_STREAMER)

/**
 * @brief Magic number used to check metadata presence
 */
#define FRAME_MAGIC_NUMBER 0xD1CECA5F

/**
 * @brief Device camera name
 */
#define DEVICE_CAMERA_NAME "/dev/mfis_cam%d"

/**
 * @brief Device blender name
 */
#define DEVICE_BLENDER_NAME "/dev/mfis_O%d"

/**
 * @brief Device camera name maximum length
 */
#define DEVICE_CAMERA_MAX_LENGTH 20

/**
 * @brief Device blender name maximum length
 */
#define DEVICE_BLENDER_MAX_LENGTH 20

/**
 * @brief FPS minium value
 */
#define FPS_MIN_VALUE 2

/**
 * @brief FPS default value
 */
#define FPS_DEFAULT_VALUE 30

/**
 * @brief FPS maximum value
 */
#define FPS_MAX_VALUE 60

/******************************************************************************************
 * Private Structures
 ******************************************************************************************/
/* Structures or enum used for internal lib purpose.
 Doesn't need to be exposed in API */

/**
 * @typedef device_type_t
 * @brief Device type
 *
 * @enum device_type
 * @brief Device type
 */
typedef enum device_type {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_CAMERA,
    DEVICE_TYPE_STREAMER,
    DEVICE_TYPE_BLENDER,
    DEVICE_TYPE_CAMERA_SEEK,
} device_type_t;

/**
 * @typedef argp_t
 * @brief argp structure typedef
 */
typedef struct argp argp_t;

/**
 * @typedef argp_state_t
 * @brief argp_state structure typedef
 */
typedef struct argp_state argp_state_t;

/**
 * @typedef argp_option_t
 * @brief argp_option structure typedef*
 */
typedef struct argp_option argp_option_t;

/**
 * @typedef sockaddr_t
 * @brief sockaddr structure typedef*
 */
typedef struct sockaddr sockaddr_t;

/**
 * @typedef sockaddr_un_t
 * @brief sockaddr_un structure typedef*
 */
typedef struct sockaddr_un sockaddr_un_t;

/**
 * @typedef pollfd_t
 * @brief pollfd structure typedef*
 */
typedef struct pollfd pollfd_t;

/**
 * @typedef dirent_t
 * @brief dirent structure typedef*
 */
typedef struct dirent dirent_t;

/**
 * @typedef stat_t
 * @brief stat structure typedef*
 */
typedef struct stat stat_t;

/**
 * @typedef device_attributes_t
 * @brief Device attributes of the device
 *
 * @struct device_attributes
 * @brief Device attributes of the device
 */
typedef struct device_attributes {
    device_type_t type;   /*!< Type */
    uint32_t buffer_size; /*!< Buffer size */
    uint32_t width;       /*!< Width */
    uint32_t height;      /*!< Height */
    uint16_t dt;          /*!< DT */
} device_attributes_t;

/**
 * @typedef device_operations_t
 * @brief Device operation that could be customized according to the device type
 *
 * @struct device_operations
 * @brief Device operation that could be customized according to the device type
 */
typedef struct device_operations {
    eviewitf_ret_t (*open)(int device_id);        /*!< Operation to be performed on open request */
    eviewitf_ret_t (*close)(int file_descriptor); /*!< Operation to be performed on close request */
    eviewitf_ret_t (*write)(int file_descriptor, uint8_t *frame_buffer,
                            uint32_t buffer_size); /*!< Called when a frame is written to a device */
    eviewitf_ret_t (*read)(int file_descriptor, uint8_t *frame_buffer,
                           uint32_t buffer_size); /*!< Called when a frame is read from a device */
    eviewitf_ret_t (*display)(int device_id);     /*!< Called when the device is selected for display */
    eviewitf_ret_t (*get_attributes)(int device_id,
                                     eviewitf_device_attributes_t *attributes); /*!< Get device attributes */

} device_operations_t;

/**
 * @typedef device_object_t
 * @brief Camera definition containing both attributes and operations for a device
 *
 * @struct device_object
 * @brief Camera definition containing both attributes and operations for a device
 */
typedef struct device_object {
    device_attributes_t attributes; /*!< Device attributes */
    device_operations_t operations; /*!< Device operations */
} device_object_t;

/******************************************************************************************
 * Private Functions Prototypes
 ******************************************************************************************/
eviewitf_ret_t eviewitf_app_reset_camera(int cam_id);
eviewitf_ret_t eviewitf_app_record_cam(int cam_id, int delay, char *record_path);
eviewitf_ret_t eviewitf_app_streamer_play(int cam_id, int fps, char *frames_dir);
eviewitf_ret_t eviewitf_app_set_blending_from_file(int blender_id, char *frame);
eviewitf_ret_t eviewitf_app_print_monitoring_info(void);

/* Common */
eviewitf_ret_t eviewitf_is_initialized();

/* Devices */
eviewitf_ret_t device_objects_init();
device_object_t *get_device_object(int device_id);
eviewitf_ret_t device_get_attributes(int device_id, eviewitf_device_attributes_t *attributes);
eviewitf_ret_t device_open(int device_id);
eviewitf_ret_t device_close(int device_id);
eviewitf_ret_t device_seek(int device_id, off_t offset, int whence);
eviewitf_ret_t device_read(int device_id, uint8_t *frame_buffer, uint32_t buffer_size);
eviewitf_ret_t device_write(int device_id, uint8_t *frame_buffer, uint32_t buffer_size);
eviewitf_ret_t device_poll(int *device_id, int nb_devices, int ms_timeout, short *event_return);

/* Blender */
eviewitf_ret_t blender_open(int device_id);

/* Camera */
eviewitf_ret_t camera_open(int device_id);
eviewitf_ret_t camera_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);
eviewitf_ret_t camera_display(int device_id);

/* Streamer */
eviewitf_ret_t streamer_open(int device_id);

/* Camera Seek */
/**
 * @fn inteviewitf_ret_t camera_seek_register(int cam_id)
 * @brief register a seek camera
 *
 * @param cam_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 *
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t camera_seek_register(int cam_id);

/**
 * @fn int camera_seek_open(int cam_id)
 * @brief open a seek camera device
 *
 * @param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 *        we assume this value has been tested by the caller
 *
 * @return file descriptor or -1
 */
eviewitf_ret_t camera_seek_open(int device_id);

/**
 * @fn int camera_seek_close(int file_descriptor)
 * @brief close seek camera device
 *
 * @param file_descriptor: file descriptor on an opened device
 *
 * @return 0 on success otherwise -1
 */
eviewitf_ret_t camera_seek_close(int file_descriptor);

/**
 * @fn int camera_seek_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size)
 * @brief Read from a seek camera
 *
 * @param file_descriptor: file descriptor on an opened device
 * @param frame_buffer: buffer containing the frame
 * @param buffer_size: size of the frame
 *
 * @return the number of read bytes or -1
 */
eviewitf_ret_t camera_seek_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);

/**
 * @fn eviewitf_ret_t camera_seek_display(int cam_id)
 * @brief Request R7 to select camera device as display input
 *
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t camera_seek_display(int device_id);

/**
 * @fn eviewitf_ret_t camera_seek_get_attributes(int device_id, eviewitf_device_attributes_t *attributes)
 * @brief Get seek camera attributes
 *
 * @param device_id: id of the Seek device
 * @param attributes: attributes structure to be filled in
 *
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t camera_seek_get_attributes(int device_id, eviewitf_device_attributes_t *attributes);

#endif /* EVIEWITF_PRIV_H */

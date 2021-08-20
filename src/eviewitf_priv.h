/**
 * \file
 * \brief Internal header for private structures
 * \author LACROIX Impulse
 */

#ifndef EVIEWITF_PRIV_H
#define EVIEWITF_PRIV_H

#include <stdint.h>

#include "eviewitf.h"

/******************************************************************************************
 * Private Definitions
 ******************************************************************************************/

#define EVIEWITF_MAX_DEVICES     (EVIEWITF_MAX_CAMERA + EVIEWITF_MAX_STREAMER + EVIEWITF_MAX_BLENDER)
#define EVIEWITF_OFFSET_CAMERA   (0)
#define EVIEWITF_OFFSET_STREAMER (EVIEWITF_MAX_CAMERA)
#define EVIEWITF_OFFSET_BLENDER  (EVIEWITF_OFFSET_STREAMER + EVIEWITF_MAX_STREAMER)

/* Magic number used to check metadata presence */
#define FRAME_MAGIC_NUMBER 0xD1CECA5F

#define DEVICE_CAMERA_NAME        "/dev/mfis_cam%d"
#define DEVICE_BLENDER_NAME       "/dev/mfis_O%d"
#define DEVICE_CAMERA_MAX_LENGTH  20
#define DEVICE_BLENDER_MAX_LENGTH 20

/******************************************************************************************
 * Private Structures
 ******************************************************************************************/
/* Structures or enum used for internal lib purpose.
 Doesn't need to be exposed in API */

/**
 * @brief Device type
 */
typedef enum {
    DEVICE_TYPE_NONE,
    DEVICE_TYPE_CAMERA,
    DEVICE_TYPE_STREAMER,
    DEVICE_TYPE_BLENDER,
    DEVICE_TYPE_CAMERA_SEEK,
} device_type;

/**
 * @brief Device attributes
 * Defines some attributes of the device
 */
typedef struct {
    device_type type;
    uint32_t buffer_size;
    uint32_t width;
    uint32_t height;
    uint16_t dt;
} device_attributes;

/**
 * @brief Device operation
 * Defines some operation that could be customized according to the device type
 */
typedef struct {
    /* Operation to be performed on open request */
    int (*open)(int device_id);

    /* Operation to be performed on close request */
    int (*close)(int file_descriptor);

    /* Called when a frame is written to a device */
    int (*write)(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);

    /* Called when a frame is read from a device */
    int (*read)(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);

    /* Called when the device is selected for display */
    int (*display)(int device_id);

    /* Get device attributes */
    int (*get_attributes)(int device_id, eviewitf_device_attributes_t *attributes);
} device_operations;

/**
 * @brief Camera definition
 * Contains both attributes and operations for a device
 */
typedef struct {
    /* Device attributes */
    device_attributes attributes;

    /* Device operations */
    device_operations operations;
} device_object;

/******************************************************************************************
 * Private Functions Prototypes
 ******************************************************************************************/

/* App eViewItf */
int eviewitf_app_reset_camera(int cam_id);
int eviewitf_app_record_cam(int cam_id, int delay, char *record_path);
int eviewitf_app_streamer_play(int cam_id, int fps, char *frames_dir);
int eviewitf_app_set_blending_from_file(int blender_id, char *frame);
int eviewitf_app_print_monitoring_info(void);

/* Common */
int eviewitf_is_initialized();

/* Devices */
int device_objects_init();
device_object *get_device_object(int device_id);
int device_get_attributes(int device_id, eviewitf_device_attributes_t *attributes);
int device_open(int device_id);
int device_close(int device_id);
int device_read(int device_id, uint8_t *frame_buffer, uint32_t buffer_size);
int device_write(int device_id, uint8_t *frame_buffer, uint32_t buffer_size);
int device_poll(int *device_id, int nb_devices, int ms_timeout, short *event_return);

/* Blender */
int blender_open(int device_id);

/* Camera */
int camera_open(int device_id);
int camera_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);
int camera_display(int device_id);

/* Streamer */
int streamer_open(int device_id);

/* Camera Seek */
int camera_seek_register(int cam_id);
int camera_seek_open(int device_id);
int camera_seek_close(int file_descriptor);
int camera_seek_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);
int camera_seek_display(int device_id);
int camera_seek_get_attributes(int device_id, eviewitf_device_attributes_t *attributes);

#endif /* EVIEWITF_PRIV_H */

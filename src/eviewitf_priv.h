/**
 * \file eviewitf-priv.h
 * \brief Internal header for private structures
 * \author esoftthings
 */

#ifndef EVIEWITF_PRIV_H
#define EVIEWITF_PRIV_H

#include <stdint.h>
#include "mfis_communication.h"

/******************************************************************************************
 * Private Definitions
 ******************************************************************************************/

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
 * \enum fct_ret_r
 * \brief Return codes
 */
typedef enum {
    FCT_RETURN_OK = 1,
    FCT_RETURN_BLOCKED,
    FCT_INV_PARAM,
    FCT_RETURN_ERROR,
} fct_ret_r;

/**
 * @brief Camera operation
 * Defines some operation that could be customized according to the camera type
 */
typedef struct eviewitf_camera_operations {
    /* Operation to be performed on open request */
    int (*open)(int cam_id);

    /* Operation to be performed on close request */
    int (*close)(int file_descriptor);

    /* Called when a frame is written to a camera */
    int (*write)(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);

    /* Called when a frame is read fro ma camera */
    int (*read)(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);

    /* Called when the camera is selected for display */
    int (*display)(int cam_id);
} eviewitf_camera_operations_t;

/**
 * @brief Camera definition
 * Contains both attributes and operations for a camera
 */
typedef struct eviewitf_camera_object {
    /* Camera attributes from MFIS */
    struct eviewitf_mfis_camera_attributes camera_attributes;

    /* Camera operations related tio camera type */
    struct eviewitf_camera_operations camera_operations;
} eviewitf_camera_object_t;

/******************************************************************************************
 * Private Functions Prototypes
 ******************************************************************************************/

/* App eViewItf */
int eviewitf_app_reset_camera(int cam_id);
int eviewitf_app_record_cam(int cam_id, int delay, char *record_path);
int eviewitf_app_streamer_play(int cam_id, int fps, char *frames_dir);
int eviewitf_app_set_blending_from_file(int blender_id, char *frame);

/* Common */
int eviewitf_is_initialized();
struct eviewitf_camera_object *eviewitf_get_camera_object(int cam_id);
struct eviewitf_mfis_blending_attributes *eviewitf_get_blender_attributes(int cam_id);

/* To be moved ? */
int camera_generic_open(int cam_id);
int camera_generic_close(int file_descriptor);
int camera_generic_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);
int generic_camera_display(int cam_id);

int camera_streamer_open(int cam_id);
int camera_streamer_close(int file_descriptor);
int camera_streamer_write(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size);


#endif /* EVIEWITF_PRIV_H */

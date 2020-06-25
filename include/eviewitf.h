/**
 * \file eviewitf.h
 * \brief Header for Communication API between A53 and R7 CPUs
 * \author esoftthings
 */

#ifndef EVIEWITF_H
#define EVIEWITF_H

#include <stdint.h>

/******************************************************************************************
 * Public Definitions
 ******************************************************************************************/
#define EVIEWITF_MAX_CAMERA      8
#define EVIEWITF_MAX_STREAMER    8
#define EVIEWITF_MAX_BLENDING    2

/******************************************************************************************
 * Public Structures
 ******************************************************************************************/
/**
 * \enum eviewitf_return_state
 * \brief Return Codes
 */

typedef enum {
    EVIEWITF_OK,
    EVIEWITF_BLOCKED = -1,
    EVIEWITF_INVALID_PARAM = -2,
    EVIEWITF_NOT_INITIALIZED = -3,
    EVIEWITF_NOT_OPENED = -4,
    EVIEWITF_FAIL = -5,
} eviewitf_return_state;

/**
 * \struct eviewitf_frame_metadata_info_t
 * \brief Pointers to current camera frame metadata
 *
 */
typedef struct {
    uint32_t frame_width;
    uint32_t frame_height;
    uint32_t frame_bpp;
    uint32_t frame_timestamp_lsb;
    uint32_t frame_timestamp_msb;
    uint32_t frame_sync;
    uint32_t reserved[24]; /* 32 metadata fields in total*/
    uint32_t frame_size;
    uint32_t magic_number;
} eviewitf_frame_metadata_info_t;

/**
 * \struct eviewitf_device_attributes_t
 * \brief Structure to get a device (camera, streamer or blender) attributes
 */
typedef struct {
    uint32_t buffer_size;
    uint32_t width;
    uint32_t height;
    uint16_t dt;
} eviewitf_device_attributes_t;

/******************************************************************************************
 * Public Functions Prototypes
 ******************************************************************************************/

/* eView */
int eviewitf_init(void);
int eviewitf_deinit(void);
int eviewitf_set_R7_heartbeat_mode(uint32_t mode);
int eviewitf_set_R7_boot_mode(uint32_t mode);

/* Version */
const char* eviewitf_get_eview_version(void);
const char* eviewitf_get_eviewitf_version(void);

/* Cameras */
int eviewitf_camera_open(int cam_id);
int eviewitf_camera_close(int cam_id);
int eviewitf_camera_get_attributes(int cam_id, eviewitf_device_attributes_t *attributes);
int eviewitf_camera_get_frame(int cam_id, uint8_t* frame_buffer, uint32_t buffer_size);
int eviewitf_camera_extract_metadata(uint8_t* buf, uint32_t buffer_size,
                                     eviewitf_frame_metadata_info_t* frame_metadata);
int eviewitf_camera_poll(int* cam_id, int nb_cam, short* event_return);
int eviewitf_camera_get_parameter(int cam_id, int cam_type, uint32_t reg_address, uint32_t* reg_value);
int eviewitf_camera_set_parameter(int cam_id, int cam_type, uint32_t reg_address, uint32_t reg_value);

/* Streamer */
int eviewitf_streamer_open(int streamer_id);
int eviewitf_streamer_close(int streamer_id);
int eviewitf_streamer_get_attributes(int streamer_id, eviewitf_device_attributes_t *attributes);
int eviewitf_streamer_write_frame(int streamer_id, uint32_t buffer_size, char* buffer);

/* Blending */
int eviewitf_write_blending(int blending_id, uint32_t buffer_size, char* buffer);
int eviewitf_start_blending(int blending_id);
int eviewitf_stop_blending(void);

/* Cropping */
int eviewitf_set_display_cam(int cam_id);
int eviewitf_start_cropping(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
int eviewitf_stop_cropping(void);


/* Specific SEEK */
int eviewitf_import_seek_plugin(void);
char* eviewitf_seek_get_plugin_version(void);
char* eviewitf_seek_get_seek_version(void);
int eviewitf_seek_init_all_cameras(int nb_cam);
int eviewitf_seek_deinit_all_cameras(int nb_cam);
int eviewitf_seek_start_camera(int cam_id);
int eviewitf_seek_stop_camera(int cam_id);
int eviewitf_seek_get_camera_setting(int cam_id, int setting_nb, int* setting_value);
int eviewitf_seek_set_camera_setting(int cam_id, int setting_nb, int setting_value);
int eviewitf_seek_get_camera_frame(int cam_id, float** temperature, uint32_t** display);
#endif /* EVIEWITF_H */

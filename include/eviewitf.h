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
#define EVIEWITF_MAX_CAMERA      16
#define EVIEWITF_MAX_REAL_CAMERA 8

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
    EVIEWITF_FAIL = -3,
} eviewitf_return_state;

/**
 * \struct eviewitf_cam_buffers_info_t
 * \brief Pointers to current camera frame buffer
 *
 */
typedef struct {
    uint32_t buffer_size;
    uint8_t* ptr_buf;
} eviewitf_frame_buffer_info_t;

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

static const char* mfis_device_filenames[EVIEWITF_MAX_CAMERA] = {
    "/dev/mfis_cam0",  "/dev/mfis_cam1",  "/dev/mfis_cam2",  "/dev/mfis_cam3", "/dev/mfis_cam4",  "/dev/mfis_cam5",
    "/dev/mfis_cam6",  "/dev/mfis_cam7",  "/dev/mfis_cam8",  "/dev/mfis_cam9", "/dev/mfis_cam10", "/dev/mfis_cam11",
    "/dev/mfis_cam12", "/dev/mfis_cam13", "/dev/mfis_cam14", "/dev/mfis_cam15"};

/******************************************************************************************
 * Public Functions Prototypes
 ******************************************************************************************/
int eviewitf_get_frame(int cam_id, eviewitf_frame_buffer_info_t* frame_buffer,
                       eviewitf_frame_metadata_info_t* frame_metadata);
int eviewitf_init_api(void);
int eviewitf_deinit_api(void);
int eviewitf_set_display_cam(int cam_id);
int eviewitf_record_cam(int cam_id, int delay);
int eviewitf_get_camera_param(int cam_id, int cam_type, int reg_adress, uint16_t* reg_value);
int eviewitf_set_camera_param(int cam_id, int cam_type, int reg_adress, int reg_value);
int eviewitf_reboot_cam(int cam_id);
int eviewitf_virtual_cam_update(int cam_id, int fps, char* frames_dir);
int eviewitf_poll(int* cam_id, int nb_cam, short* event_return);
int eviewitf_set_camera_fps(int cam_id, uint32_t fps);
#endif /* EVIEWITF_H */

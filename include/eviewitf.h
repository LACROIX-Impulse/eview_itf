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
    EVIEWITF_FAIL = -3,
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

static const char* blending_interface[EVIEWITF_MAX_BLENDING] = {
    "/dev/mfis_O2",
    "/dev/mfis_O3",
};
static const char* mfis_device_filenames[EVIEWITF_MAX_CAMERA] = {
    "/dev/mfis_cam0",  "/dev/mfis_cam1",  "/dev/mfis_cam2",  "/dev/mfis_cam3", "/dev/mfis_cam4",  "/dev/mfis_cam5",
    "/dev/mfis_cam6",  "/dev/mfis_cam7",  "/dev/mfis_cam8",  "/dev/mfis_cam9", "/dev/mfis_cam10", "/dev/mfis_cam11",
    "/dev/mfis_cam12", "/dev/mfis_cam13", "/dev/mfis_cam14", "/dev/mfis_cam15"};

/******************************************************************************************
 * Public Functions Prototypes
 ******************************************************************************************/

/* eView */
int eviewitf_init_api(void);
int eviewitf_deinit_api(void);
int eviewitf_set_R7_heartbeat_mode(uint32_t mode);
int eviewitf_set_R7_boot_mode(uint32_t mode);
const char* eviewitf_get_eview_version(void);

/* Cameras */
int eviewitf_camera_open(int cam_id);
int eviewitf_camera_close(int cam_id);
int eviewitf_check_camera_on(int cam_id);
uint32_t eviewitf_camera_get_buffer_size(int cam_id);
int eviewitf_camera_get_frame(int cam_id, uint8_t* frame_buffer, uint32_t buffer_size);
int eviewitf_set_virtual_cam(int cam_id, uint32_t buffer_size, char* buffer);
int eviewitf_poll(int* cam_id, int nb_cam, short* event_return);
int eviewitf_get_camera_param(int cam_id, int cam_type, int reg_adress, uint16_t* reg_value);
int eviewitf_set_camera_param(int cam_id, int cam_type, int reg_adress, int reg_value);
int eviewitf_set_camera_fps(int cam_id, uint32_t fps);
int eviewitf_camera_extract_metadata(uint8_t* buf, uint32_t buffer_size,
                                     eviewitf_frame_metadata_info_t* frame_metadata);
int eviewitf_set_display_cam(int cam_id);

/* Blending */
int eviewitf_write_blending(int blending_id, uint32_t buffer_size, char* buffer);
int eviewitf_start_blending(int blending_id);
int eviewitf_stop_blending(void);

/* Cropping */
int eviewitf_send_cropping(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2);
int eviewitf_stop_cropping(void);
/* Infos */
const char* eviewitf_get_lib_version(void);
#endif /* EVIEWITF_H */

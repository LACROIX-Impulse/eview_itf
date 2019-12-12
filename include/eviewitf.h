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
#define EVIEWITF_MAX_CAMERA 8

/******************************************************************************************
 * Public Structures
 ******************************************************************************************/
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
    uint64_t frame_timestamp;
} eviewitf_frame_metadata_info_t;

/******************************************************************************************
 * Public Functions Prototypes
 ******************************************************************************************/
int eviewitf_get_frame(int cam_id, eviewitf_frame_buffer_info_t* frame_buffer,
                       eviewitf_frame_metadata_info_t* frame_metadata);
int eviewitf_init_api(void);
int eviewitf_deinit_api(void);
int eviewitf_set_display_cam(int cam_id);
int eviewitf_record_cam(int cam_id, int delay);
#endif /* EVIEWITF_H */

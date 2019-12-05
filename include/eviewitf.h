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
 * \brief Pointers to a camera triple buffers
 *
 */
typedef struct {
    uint32_t buffer_size;
    uint8_t* ptr_buf[3];
} eviewitf_cam_buffers_info_t;

/**
 * \struct eviewitf_cam_buffers_t
 * \brief Pointers to all camera buffers
 *
 */
typedef struct {
    eviewitf_cam_buffers_info_t cam[EVIEWITF_MAX_CAMERA];
} eviewitf_cam_buffers_t;

/******************************************************************************************
 * Public Functions Prototypes
 ******************************************************************************************/
int eviewitf_get_cam_buffers(eviewitf_cam_buffers_t* cam_buffers);
int eviewitf_init_api(void);
int eviewitf_deinit_api(void);
#endif /* EVIEWITF_H */

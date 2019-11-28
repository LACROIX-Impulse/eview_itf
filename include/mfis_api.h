/**
 * \file mfis_api.h
 * \brief Header for Communication API between A53 and R7 CPUs
 * \author esoftthings
 */

#ifndef MFIS_API_H
#define MFIS_API_H

#include <stdint.h>

/******************************************************************************************
* Public Definitions
******************************************************************************************/
#define MFIS_API_MAX_CAMERA  8

/******************************************************************************************
* Public Structures
******************************************************************************************/
/**
 * \struct mfis_api_cam_buffers_info_t
 * \brief Pointers to a camera triple buffers
 *
 */
typedef struct
{
   uint32_t buffer_size;
   uint8_t *ptr_buf[3];
}
mfis_api_cam_buffers_info_t;

/**
 * \struct mfis_api_cam_buffers_t
 * \brief Pointers to all camera buffers
 *
 */
typedef struct
{
    mfis_api_cam_buffers_info_t cam[MFIS_API_MAX_CAMERA];
}
mfis_api_cam_buffers_t;

/******************************************************************************************
* Public Functions Prototypes
******************************************************************************************/
int mfis_get_cam_buffers(mfis_api_cam_buffers_t* cam_buffers);
int mfis_init_api(void);
int mfis_deinit_api(void);
#endif /* MFIS_API_H */


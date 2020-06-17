/**
 * \file mfis_communication.h
 * \brief Header for communication with MFIS kernel driver
 * \author esoftthings
 */

#ifndef MFIS_COMMUNICATION_H
#define MFIS_COMMUNICATION_H

#include <stdint.h>

/******************************************************************************************
 * Typedef definitions
 ******************************************************************************************/
/**
 * \enum eviewitf_camera_type
 * \brief Enumerate the different types of cameras possible
 */
typedef enum {
    CAM_TYPE_NONE,
    CAM_TYPE_GENERIC,
    CAM_TYPE_VIRTUAL,
    CAM_TYPE_SEEK,
} eviewitf_camera_type;

/**
 * \struct mfis_camera_attributes
 * \brief Structure to get the cameras attributes through an ioctl call
 */
typedef struct {
    uint8_t cam_type;
    uint32_t buffer_size;
    uint32_t width;
    uint32_t height;
    uint16_t dt;
} mfis_camera_attributes;

/**
 * \struct mfis_blending_attributes
 * \brief Structure to get the blending attributes through an ioctl call
 */
typedef struct {
    uint32_t buffer_size;
    uint32_t width;
    uint32_t height;
    uint16_t dt;
} mfis_blending_attributes;

/******************************************************************************************
 * Public Definitions
 ******************************************************************************************/
/**  \def  MFIS_MSG_SIZE: number of 32bits word per MFIS request */
#define MFIS_MSG_SIZE 8

/* IOCTL definitions */
#define WR_VALUE              _IOW('a', 1, int32_t *)
#define RD_VALUE              _IOR('a', 2, int32_t *)
#define MFIS_CAM_ATTRIBUTES   _IOR('a', 3, mfis_camera_attributes *)
#define MFIS_BLEND_ATTRIBUTES _IOR('a', 4, mfis_blending_attributes *)

/******************************************************************************************
 * Public Functions Prototypes
 ******************************************************************************************/
int mfis_init();
int mfis_deinit();
int mfis_send_request(int32_t *send, int32_t *receive);
int mfis_get_cam_attributes(mfis_camera_attributes *cameras_attributes);
int mfis_get_blend_attributes(mfis_blending_attributes *blendings_attributes);
void *mfis_get_virtual_address(const uint32_t physical_address, uint32_t mem_size);
#endif /* MFIS_COMMUNICATION_H */

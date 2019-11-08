/**
 * \file mfis_api.c
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
 * \struct Str_t
 * \brief Objet chaîne de caractères.
 *
 * Str_t est un petit objet de gestion de chaînes de caractères. 
 * La chaîne se termine obligatoirement par un zéro de fin et l'objet 
 * connait la taille de chaîne contient !
 */
typedef struct
{
   uint32_t buffer_size;
   uint32_t *ptr_buf1;
   uint32_t *ptr_buf2;
   uint32_t *ptr_buf3;
}
mfis_api_cam_buffers_info_t;

typedef struct
{
    mfis_api_cam_buffers_info_t cam[MFIS_API_MAX_CAMERA];
}
mfis_api_cam_buffers_t;

typedef struct
{
    uint32_t last_buffer_id[MFIS_API_MAX_CAMERA];
}
mfis_api_cam_last_buffers_id_t;

/******************************************************************************************
* Public Functions Prototypes
******************************************************************************************/
int mfis_get_cam_buffers(mfis_api_cam_buffers_t* cam_buffers);
mfis_api_cam_last_buffers_id_t* mfis_get_cam_last_buffers_id(void);

#endif /* MFIS_API_H */


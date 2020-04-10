/**
 * \file eviewitf-priv.h
 * \brief Internal header for private structures
 * \author esoftthings
 */

#ifndef EVIEWITF_PRIV_H
#define EVIEWITF_PRIV_H

#include <stdint.h>

/******************************************************************************************
 * Private Definitions
 ******************************************************************************************/

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
 * \struct eviewitf_cam_buffers_virtual_t
 * \brief Buffer size and address in virtual memory
 */
typedef struct {
    uint32_t buffer_size;
    uint8_t *buffer;
} eviewitf_cam_buffers_virtual_t;

/**
 * \struct eviewitf_cam_buffers_a53_t
 * \brief Structure with buffers size and address in virtual memory of multiple devices
 */
typedef struct {
    eviewitf_cam_buffers_virtual_t cam[EVIEWITF_MAX_CAMERA];
    eviewitf_cam_buffers_virtual_t O2;
    eviewitf_cam_buffers_virtual_t O3;
} eviewitf_cam_buffers_a53_t;


/******************************************************************************************
 * Private Functions Prototypes
 ******************************************************************************************/

#endif /* EVIEWITF_PRIV_H */

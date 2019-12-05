/**
 * \file eviewitf.c
 * \brief Communication API between A53 and R7 CPUs
 * \author esoftthings
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <signal.h>
#include "mfis_communication.h"
#include "eviewitf.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/
/* Function returned state */
#define FCT_RETURN_OK 1

/******************************************************************************************
 * Private structures
 ******************************************************************************************/
/* Structures used for internal communication between A53 and R7.
   Doesn't need to be exposed in API */
typedef struct {
    uint32_t buffer_size;
    uint32_t ptr_buf[3];
} eviewitf_cam_buffers_info_r7_t;

typedef struct {
    eviewitf_cam_buffers_info_r7_t cam[EVIEWITF_MAX_CAMERA];
} eviewitf_cam_buffers_r7_t;

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/
typedef enum {
    FCT_GET_CAM_BUFFERS,
    FCT_INIT_API,
    FCT_DEINIT_API,
    FCT_SET_DISPLAY_CAM,
    NB_FCT,
} fct_id_t;

/******************************************************************************************
 * Functions
 ******************************************************************************************/
/**
 * \fn int mfis_get_cam_buffers(eviewitf_cam_buffers_t* cam_buffers)
 * \brief Return pointers to the cameras frame buffers located in R7 memory
 *
 * \param eviewitf_cam_buffers_t* cam_buffers: structure that will be filled with frame buffers pointers
 * \return state of the function. Return 0 if okay
 */
int eviewitf_get_cam_buffers(eviewitf_cam_buffers_t* cam_buffers) {
    int ret = 0, i;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];
    eviewitf_cam_buffers_r7_t* cam_buffers_r7;

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_GET_CAM_BUFFERS;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if (ret < 0) {
        ret = -1;
        goto out;
    }

    /* Check returned answer state */
    if ((rx_buffer[0] != FCT_GET_CAM_BUFFERS) && (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = -1;
        goto out;
    }

    /* R7 return a pointer to a structure stored in its memory. Convert this pointer into a virtual adress for A53 */
    cam_buffers_r7 =
        (eviewitf_cam_buffers_r7_t*)mfis_get_virtual_address(rx_buffer[2], sizeof(eviewitf_cam_buffers_r7_t));

    /* Fill the A53 cam_buffer structure with value returned by R7*/
    for (i = 0; i < EVIEWITF_MAX_CAMERA; i++) {
        cam_buffers->cam[i].buffer_size = cam_buffers_r7->cam[i].buffer_size;

        if (cam_buffers->cam[i].buffer_size != 0) {
            /* Convert R7 physical addresses of frame buffers to virtual adresses */
            cam_buffers->cam[i].ptr_buf[0] =
                mfis_get_virtual_address(cam_buffers_r7->cam[i].ptr_buf[0], cam_buffers_r7->cam[i].buffer_size);
            cam_buffers->cam[i].ptr_buf[1] =
                mfis_get_virtual_address(cam_buffers_r7->cam[i].ptr_buf[1], cam_buffers_r7->cam[i].buffer_size);
            cam_buffers->cam[i].ptr_buf[2] =
                mfis_get_virtual_address(cam_buffers_r7->cam[i].ptr_buf[2], cam_buffers_r7->cam[i].buffer_size);
        }
    }

out:
    return ret;
}

/**
 * \fn eviewitf_init_api
 * \brief Deinit MFIS driver on R7 side
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_init_api(void) {
    int ret = 0;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_INIT_API;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if (ret < 0) {
        ret = -1;
        goto out;
    }

    /* Check returned answer state */
    if ((rx_buffer[0] != FCT_INIT_API) && (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = -1;
        goto out;
    }

out:
    return ret;
}

/**
 * \fn eviewitf_deinit_api
 * \brief Deinit MFIS driver on R7 side
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_deinit_api(void) {
    int ret = 0;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_DEINIT_API;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if (ret < 0) {
        ret = -1;
        goto out;
    }

    /* Check returned answer state */
    if ((rx_buffer[0] != FCT_DEINIT_API) && (rx_buffer[1] != FCT_RETURN_OK)) {
        ret = -1;
        goto out;
    }

out:
    return ret;
}

/**
 * \fn eviewitf_set_display_cam
 * \brief Request R7 to change camera used on display
 *
 * \return state of the function. Return 0 if okay
 */
int eviewitf_set_display_cam(int cam_id) {
    int ret = 0;
    int32_t tx_buffer[MFIS_MSG_SIZE], rx_buffer[MFIS_MSG_SIZE];

    memset(tx_buffer, 0, sizeof(tx_buffer));
    memset(rx_buffer, 0, sizeof(rx_buffer));

    /* Prepare TX buffer */
    tx_buffer[0] = FCT_SET_DISPLAY_CAM;

    /* Send request to R7 */
    ret = mfis_send_request(tx_buffer, rx_buffer);
    if (ret < 0) {
        ret = -1;
    } else {
        /* Check returned answer state */
        if ((rx_buffer[0] != FCT_SET_DISPLAY_CAM) && (rx_buffer[1] != FCT_RETURN_OK)) {
            ret = -1;
        }
    }

    return ret;
}

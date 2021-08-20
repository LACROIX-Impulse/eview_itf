/**
 * \file mfis_communication.c
 * \brief Handle communication with kernel
 * \author LACROIX Impulse
 *
 * Handle low level communication with Linux kernel drivers
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
#include <sys/mman.h>
#include <pthread.h>

#include "eviewitf.h"
#include "mfis_communication.h"
#include "mfis-ioctl.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/
pthread_mutex_t mfis_mutex;

/******************************************************************************************
 * Functions
 ******************************************************************************************/

int mfis_init() { return pthread_mutex_init(&mfis_mutex, NULL); }

int mfis_deinit() { return pthread_mutex_destroy(&mfis_mutex); }

/**
 * \fn int mfis_send_request(int32_t* request)
 * \brief Send a request to R7 CPU and return its answer.
 *
 * \param request: array of 32bits containing in/out message to/from R7
 * \return state of the function. Return 0 if okay
 */
int mfis_send_request(int32_t* request) {
    int fd, ret;

    pthread_mutex_lock(&mfis_mutex);
    /* Open MFIS IOCTL */
    fd = open("/dev/mfis_ioctl", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "%s() error cannot open ioctl file : %s\n", __FUNCTION__, strerror(errno));
        ret = fd;
        goto out_ret;
    }

    /* Send message over MFIS */
    ret = ioctl(fd, EVIEWITF_MFIS_FCT, (int32_t*)request);
    if (ret < 0) {
        fprintf(stderr, "%s() ioctl write error : %s\n", __FUNCTION__, strerror(errno));
    }

    close(fd);
out_ret:
    pthread_mutex_unlock(&mfis_mutex);
    return ret;
}

/**
 * \fn void* mfis_get_cam_attributes(struct eviewitf_mfis_camera_attributes *cameras_attributes) {
 * \brief Get cameras attributes from MFIS
 *
 * \param [inout] cameras_attributes: Pointer to a table of struct eviewitf_mfis_camera_attributes
 *
 * \return pointer to virtual address (return NULL if error).
 */
int mfis_get_cam_attributes(struct eviewitf_mfis_camera_attributes* cameras_attributes) {
    int fd;
    int ret;
    pthread_mutex_lock(&mfis_mutex);

    /* Open the mfis device */
    fd = open("/dev/mfis_ioctl", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "%s() error cannot open ioctl file : %s\n", __FUNCTION__, strerror(errno));
        pthread_mutex_unlock(&mfis_mutex);
        return EVIEWITF_FAIL;
    }

    /* Call to the ioctl */
    ret = ioctl(fd, EVIEWITF_MFIS_CAMERA_ATTRIBUTES, cameras_attributes);
    if (ret < 0) {
        fprintf(stderr, "%s() ioctl error : %s\n", __FUNCTION__, strerror(errno));
        pthread_mutex_unlock(&mfis_mutex);
        return EVIEWITF_FAIL;
    }

    /* Close the file descriptor and release the mutex */
    close(fd);
    pthread_mutex_unlock(&mfis_mutex);
    return EVIEWITF_OK;
}

/**
 * \fn void* mfis_get_blend_attributes(struct eviewitf_mfis_blending_attributes *blendings_attributes) {
 * \brief Get blendings attributes from MFIS
 *
 * \param [inout] blendings_attributes: Pointer to a table of struct eviewitf_mfis_blending_attributes
 *
 * \return pointer to virtual address (return NULL if error).
 */
int mfis_get_blend_attributes(struct eviewitf_mfis_blending_attributes* blendings_attributes) {
    int fd;
    int ret;
    pthread_mutex_lock(&mfis_mutex);

    /* Open the mfis device */
    fd = open("/dev/mfis_ioctl", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "%s() error cannot open ioctl file : %s\n", __FUNCTION__, strerror(errno));
        pthread_mutex_unlock(&mfis_mutex);
        return EVIEWITF_FAIL;
    }

    /* Call to the ioctl */
    ret = ioctl(fd, EVIEWITF_MFIS_BLENDING_ATTRIBUTES, blendings_attributes);
    if (ret < 0) {
        fprintf(stderr, "%s() ioctl error : %s\n", __FUNCTION__, strerror(errno));
        pthread_mutex_unlock(&mfis_mutex);
        return EVIEWITF_FAIL;
    }

    /* Close the file descriptor and release the mutex */
    close(fd);
    pthread_mutex_unlock(&mfis_mutex);
    return EVIEWITF_OK;
}

/**
 * @brief Delivers an ioctl to the MFIS driver
 *
 * @param[in]  devtype       Device type
 * @param[in]  devid         Device identifier
 * @param[in]  cmd           I/O command
 * @param[in]  param         I/O parameter
 * @return EVIEWITF_OK on success, negative value on failure (see errno.h)
 */
int mfis_ioctl_request(uint8_t devtype, uint8_t devid, uint16_t cmd, void* param) {
    int fd;
    int ret = EVIEWITF_OK;
    uint32_t msg[EVIEWITF_MFIS_MSG_SIZE];
    struct mfis_ioctl* hdr;

    /* Prepares the message header */
    hdr = (struct mfis_ioctl*)msg;
    hdr->funcid = EVIEWITF_MFIS_FCT_IOCTL;
    hdr->requester = 0; /* Keep it empty, driver will set this field */
    hdr->devtype = devtype;
    hdr->devid = devid;
    hdr->result = 0;
    hdr->cmd = cmd;

    /* Copies the parameter based on the IOC message size */
    if (param && MFIS_IOCSZ(cmd) > 0) memcpy(msg + 2, param, MFIS_IOCSZ(cmd));

    pthread_mutex_lock(&mfis_mutex);
    /* Open MFIS IOCTL */
    fd = open("/dev/mfis_ioctl", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "%s() error cannot open ioctl file : %s\n", __FUNCTION__, strerror(errno));
        ret = EVIEWITF_FAIL;
        goto out_ret;
    }

    /* Send message over MFIS */
    ret = ioctl(fd, EVIEWITF_MFIS_FCT, (uint32_t*)msg);
    close(fd);
    if (ret < 0) {
        fprintf(stderr, "%s() ioctl write error : %s\n", __FUNCTION__, strerror(errno));
        ret = EVIEWITF_FAIL;
        goto out_ret;
    }

    /* Copies the parameter based on the IOC message size */
    if (param && MFIS_IOCSZ(cmd) > 0) memcpy(param, msg + 2, MFIS_IOCSZ(cmd));

    /* Check returned answer state */
    if (hdr->funcid != EVIEWITF_MFIS_FCT_IOCTL) {
        ret = EVIEWITF_FAIL;
    } else if (hdr->result == EVIEWITF_MFIS_FCT_RETURN_ERROR) {
        ret = EVIEWITF_FAIL;
    } else if (hdr->result == EVIEWITF_MFIS_FCT_INV_PARAM) {
        ret = EVIEWITF_INVALID_PARAM;
    }

out_ret:
    pthread_mutex_unlock(&mfis_mutex);
    return ret;
}
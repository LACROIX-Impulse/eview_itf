/**
 * \file mfis_communication.c
 * \brief Handle communication with kernel
 * \author esoftthings
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
 * \fn int mfis_send_request(int32_t *send, int32_t *receive)
 * \brief Send a request to R7 CPU and return its answer.
 *
 * \param send: array of 32bits containing message to send to R7
 * \param receive: array of 32bits that will be filled with R7 answer
 * \return state of the function. Return 0 if okay
 */
int mfis_send_request(int32_t* send, int32_t* receive) {
    int fd, ret;

    pthread_mutex_lock(&mfis_mutex);
    /* Open MFIS IOCTL */
    fd = open("/dev/mfis_ioctl", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "%s() error cannot open ioctl file : %s\n", __FUNCTION__, strerror(errno));
        ret = fd;
        goto out_ret;
    }

    /* Send message to MFIS */
    ret = ioctl(fd, EVIEWITF_MFIS_WR_VALUE, (int32_t*)send);
    if (ret < 0) {
        fprintf(stderr, "%s() ioctl write error : %s\n", __FUNCTION__, strerror(errno));
        goto out_close;
    }

    /* Wait for MFIS answer from R7 */
    ret = ioctl(fd, EVIEWITF_MFIS_RD_VALUE, (int32_t*)receive);
    if (ret < 0) {
        fprintf(stderr, "%s() ioctl read error : %s\n", __FUNCTION__, strerror(errno));
        goto out_close;
    }

out_close:
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
 * \fn uint32_t* mfis_get_virtual_address(const uint32_t physical_address, uint32_t mem_size)
 * \brief Convert memory physical address to virtual one so it can be accessed from userspace (READ ONLY).
 *
 * \param const uint32_t physical_address: physical address to convert
 * \param uint32_t mem_size: size of the memory to convert
 * \return pointer to virtual address (return NULL if error).
 */
void* mfis_get_virtual_address(const uint32_t physical_address, uint32_t mem_size) {
    int mem_dev;
    uint32_t* virtual_address = NULL;
    mem_dev = open("/dev/mem", O_RDONLY);
    if (mem_dev == -1) {
        fprintf(stderr, "%s() error while opening /dev/mem : %s\n", __FUNCTION__, strerror(errno));
        goto out_ret;
    }

    virtual_address = mmap(NULL, mem_size, PROT_READ, MAP_PRIVATE, mem_dev, physical_address);
    if (virtual_address == MAP_FAILED) {
        fprintf(stderr, "%s() error MAP_FAILED : %s\n", __FUNCTION__, strerror(errno));
        virtual_address = NULL;
        goto out_close;
    }

out_close:
    close(mem_dev);
out_ret:
    return virtual_address;
}

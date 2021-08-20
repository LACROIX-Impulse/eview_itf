/**
 * \file eviewitf_camera_seek.c
 * \brief Communication API between A53 and R7 CPUs for seek camera devices
 * \author LACROIX Impulse
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdio.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

#include "eviewitf_priv.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/

#define SEEK_FRAME_WIDTH 200
#define SEEK_FRAME_HEIGH 150
#define SEEK_DT          0x01F0

#define SEEK_NB_CAMERAS           4
#define SEEK_STRING_MAX_LENGTH    64
#define SEEK_CONFIG_MESSAGE_SIZE  2
#define SEEK_SOCKET_CONFIG        "/var/seek/seek_config"
#define SEEK_SOCKET_CAMERA        "/var/seek/seek_camera_%d"
#define SEEK_SEM_MUTEX_CAMERA     "/seek-mutex-camera-%d"
#define SEEK_SHARED_MEMORY_CAMERA "/seek-shared-mem-camera-%d"
#define SEEK_STREAMER_ID          7

#define SEEK_CONFIG_START_DISPLAY_CAMERA 1
#define SEEK_CONFIG_STOP_DISPLAY_CAMERA  2

/******************************************************************************************
 * Private structures
 ******************************************************************************************/

typedef struct {
    float frame[SEEK_FRAME_WIDTH * SEEK_FRAME_HEIGH];
} seek_shared_memory;

typedef struct {
    int cam_id;
    bool used;
    sem_t *mutex_sem;
    int fd_shm;
    seek_shared_memory *ptr_shm;
    int sock;
} seek_handler;

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/

/******************************************************************************************
 * Private variables
 ******************************************************************************************/

static seek_handler seek_handlers[SEEK_NB_CAMERAS] = {0};

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/**
 * \fn int camera_seek_register(int cam_id)
 * \brief register a seek camera
 *
 * \param cam_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 *
 * \return EVIEWITF_OK or EVIEWITF_FAIL if no more slot available
 */
int camera_seek_register(int cam_id) {
    for (int i = 0; i < SEEK_NB_CAMERAS; i++) {
        if (seek_handlers[i].used == false) {
            seek_handlers[i].cam_id = cam_id;
            seek_handlers[i].used = true;
            return EVIEWITF_OK;
        }
    }
    /* No more empty seek camera */
    return EVIEWITF_FAIL;
}

/**
 * \fn int camera_seek_get_seek_id(int cam_id)
 * \brief get Seek ID from camera ID
 *
 * \param cam_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 *
 * \return seek cmera id or -1
 */
static int camera_seek_get_seek_id(int cam_id) {
    for (int seek_id = 0; seek_id < SEEK_NB_CAMERAS; seek_id++) {
        if (seek_handlers[seek_id].cam_id == cam_id) {
            return seek_id;
        }
    }
    return -1;
}

/**
 * \fn int camera_seek_open(int cam_id)
 * \brief open a seek camera device
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 *        we assume this value has been tested by the caller
 *
 * \return file descriptor or -1
 */
int camera_seek_open(int cam_id) {
    char tmp_string[SEEK_STRING_MAX_LENGTH];
    int seek_id = camera_seek_get_seek_id(cam_id);
    struct sockaddr_un server_camera;

    /* Open mutual exclusion semaphores */
    snprintf(tmp_string, SEEK_STRING_MAX_LENGTH, SEEK_SEM_MUTEX_CAMERA, seek_id);
    seek_handlers[seek_id].mutex_sem = sem_open(tmp_string, 0);
    if (seek_handlers[seek_id].mutex_sem == SEM_FAILED) {
        return -1;
    }

    /* Open shared memory */
    snprintf(tmp_string, SEEK_STRING_MAX_LENGTH, SEEK_SHARED_MEMORY_CAMERA, seek_id);
    seek_handlers[seek_id].fd_shm = shm_open(tmp_string, O_RDONLY, S_IRUSR | S_IWUSR);
    if (seek_handlers[seek_id].fd_shm == -1) {
        return -1;
    }

    /* Map shared memory */
    seek_handlers[seek_id].ptr_shm =
        mmap(NULL, sizeof(seek_shared_memory), PROT_READ, MAP_SHARED, seek_handlers[seek_id].fd_shm, 0);
    if (seek_handlers[seek_id].ptr_shm == MAP_FAILED) {
        return -1;
    }

    /* Open Unix socket */
    seek_handlers[seek_id].sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (seek_handlers[seek_id].sock < 0) {
        return -1;
    }

    /* Connect to Seek service */
    server_camera.sun_family = AF_UNIX;
    snprintf(tmp_string, SEEK_STRING_MAX_LENGTH, SEEK_SOCKET_CAMERA, seek_id);
    strcpy(server_camera.sun_path, tmp_string);

    if (connect(seek_handlers[seek_id].sock, (struct sockaddr *)&server_camera, sizeof(struct sockaddr_un)) < 0) {
        close(seek_handlers[seek_id].sock);
        seek_handlers[seek_id].sock = -1;
        return -1;
    }

    return seek_handlers[seek_id].sock;
}

/**
 * \fn int camera_seek_close(int file_descriptor)
 * \brief close seek camera device
 *
 * \param file_descriptor: file descriptor on an opened device
 *
 * \return 0 on success otherwise -1
 */
int camera_seek_close(int file_descriptor) {
    for (int i = 0; i < SEEK_NB_CAMERAS; i++) {
        if (seek_handlers[i].sock == file_descriptor) {
            seek_handlers[i].sock = -1;
            return close(file_descriptor);
        }
    }

    /* File descriptor not found */
    return -1;
}

/**
 * \fn int camera_seek_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size)
 * \brief Read from a seek camera
 *
 * \param file_descriptor: file descriptor on an opened device
 * \param frame_buffer: buffer containing the frame
 * \param buffer_size: size of the frame
 *
 * \return the number of read bytes or -1
 */
int camera_seek_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size) {
    u_int8_t msg[SEEK_CONFIG_MESSAGE_SIZE];
    int min_size = MIN(buffer_size, sizeof(seek_shared_memory));

    for (int i = 0; i < SEEK_NB_CAMERAS; i++) {
        if (seek_handlers[i].sock == file_descriptor) {
            sem_wait(seek_handlers[i].mutex_sem);
            /* Copy content from shared memory */
            memcpy(frame_buffer, seek_handlers[i].ptr_shm, min_size);
            /* Read message on socket to cancel polling */
            if (read(seek_handlers[i].sock, msg, SEEK_CONFIG_MESSAGE_SIZE) != SEEK_CONFIG_MESSAGE_SIZE) {
                min_size = -1;
            }
            sem_post(seek_handlers[i].mutex_sem);
            return min_size;
        }
    }
    return -1;
}

/**
 * \fn camera_seek_display
 * \brief Request R7 to select camera device as display input
 *
 * \return state of the function. Return 0 if okay
 */
int camera_seek_display(int cam_id) {
    int sock_config;
    struct sockaddr_un server_config;
    u_int8_t msg[SEEK_CONFIG_MESSAGE_SIZE];

    /* Create config socket */;
    sock_config = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_config < 0) {
        return EVIEWITF_FAIL;
    }
    server_config.sun_family = AF_UNIX;
    strcpy(server_config.sun_path, SEEK_SOCKET_CONFIG);

    /* Establish connection */
    if (connect(sock_config, (struct sockaddr *)&server_config, sizeof(struct sockaddr_un)) < 0) {
        close(sock_config);
        return EVIEWITF_FAIL;
    }

    /* Send display message */
    msg[0] = SEEK_CONFIG_START_DISPLAY_CAMERA;
    msg[1] = camera_seek_get_seek_id(cam_id);
    if (write(sock_config, msg, SEEK_CONFIG_MESSAGE_SIZE) != SEEK_CONFIG_MESSAGE_SIZE) {
        close(sock_config);
        return EVIEWITF_FAIL;
    }
    /* Check answer */
    if (read(sock_config, msg, SEEK_CONFIG_MESSAGE_SIZE) != SEEK_CONFIG_MESSAGE_SIZE) {
        close(sock_config);
        return EVIEWITF_FAIL;
    }
    if ((msg[0] != SEEK_CONFIG_START_DISPLAY_CAMERA) || (msg[1] != 0)) {
        close(sock_config);
        return EVIEWITF_FAIL;
    }

    close(sock_config);
    return camera_display(SEEK_STREAMER_ID + EVIEWITF_OFFSET_STREAMER);
}

/**
 * \fn int camera_seek_get_attributes(int device_id, eviewitf_device_attributes_t *attributes)
 * \brief Get seek camera attributes
 *
 * \param camera_id: id of the Seek camera
 * \param attributes: attributes structure to be filled in
 *
 * \return state of the function. Return 0 if okay
 */
int camera_seek_get_attributes(__attribute__((unused)) int device_id, eviewitf_device_attributes_t *attributes) {
    attributes->buffer_size = SEEK_FRAME_WIDTH * SEEK_FRAME_HEIGH * sizeof(float);
    attributes->width = SEEK_FRAME_WIDTH;
    attributes->height = SEEK_FRAME_HEIGH;
    attributes->dt = SEEK_DT;

    return 0;
}
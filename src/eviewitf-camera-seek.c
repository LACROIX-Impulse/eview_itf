/**
 * @file eviewitf-camera-seek.c
 * @brief Communication API between A53 and R7 CPUs for seek camera devices
 * @author LACROIX Impulse
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

#include "eviewitf-priv.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/

/**
 * @brief Seek frame width
 */
#define SEEK_FRAME_WIDTH 200

/**
 * @brief Seek frame height
 */
#define SEEK_FRAME_HEIGH 150

/**
 * @brief Seek data type
 */
#define SEEK_DT 0x01F0

/**
 * @brief Seek number of cameras
 */
#define SEEK_NB_CAMERAS 4

/**
 * @brief Seek string maximum length
 */
#define SEEK_STRING_MAX_LENGTH 64

/**
 * @brief Seek configuration message size
 */
#define SEEK_CONFIG_MESSAGE_SIZE 2

/**
 * @brief Seek socket configuration
 */
#define SEEK_SOCKET_CONFIG "/var/seek/seek_config"

/**
 * @brief Seek socket camera
 */
#define SEEK_SOCKET_CAMERA "/var/seek/seek_camera_%d"

/**
 * @brief Seek semaphore mutex camera
 */
#define SEEK_SEM_MUTEX_CAMERA "/seek-mutex-camera-%d"

/**
 * @brief Seek shared memory camera
 */
#define SEEK_SHARED_MEMORY_CAMERA "/seek-shared-mem-camera-%d"

/**
 * @brief Seek streamer identifier
 */
#define SEEK_STREAMER_ID 7

/**
 * @brief Seek configuration start display camera
 */
#define SEEK_CONFIG_START_DISPLAY_CAMERA 1

/**
 * @brief Seek configuration stop display camera
 */
#define SEEK_CONFIG_STOP_DISPLAY_CAMERA 2

/******************************************************************************************
 * Private structures
 ******************************************************************************************/

/**
 * @typedef seek_shared_memory_t
 * @brief Seek shared memory
 *
 * @struct seek_shared_memory
 * @brief Seek shared memory
 */
typedef struct seek_shared_memory {
    float frame[SEEK_FRAME_WIDTH * SEEK_FRAME_HEIGH]; /*!< Seek shared memory frame */
} seek_shared_memory_t;

/**
 * @typedef seek_handler_t
 * @brief Seek handler
 *
 * @struct seek_handler
 * @brief Seek handler
 */
typedef struct seek_handler {
    int cam_id;                    /*!< Camera identifier */
    bool used;                     /*!< Usage indicator */
    sem_t *mutex_sem;              /*!< Semaphore mutex */
    int fd_shm;                    /*!< Fiel descriptor */
    seek_shared_memory_t *ptr_shm; /*!< Seek shared memory pointer */
    int sock;                      /*!< Socket */
} seek_handler_t;

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/

/******************************************************************************************
 * Private variables
 ******************************************************************************************/

/**
 * @brief Seek handlers
 */
static seek_handler_t seek_handlers[SEEK_NB_CAMERAS] = {0};

/******************************************************************************************
 * Functions
 ******************************************************************************************/

eviewitf_ret_t camera_seek_register(int cam_id) {
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
 * @fn int camera_seek_get_seek_id(int cam_id)
 * @brief get Seek ID from camera ID
 *
 * @param cam_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 *
 * @return seek cmera id or -1
 */
static int camera_seek_get_seek_id(int cam_id) {
    for (int seek_id = 0; seek_id < SEEK_NB_CAMERAS; seek_id++) {
        if (seek_handlers[seek_id].cam_id == cam_id) {
            return seek_id;
        }
    }
    return -1;
}

int camera_seek_open(int cam_id) {
    char tmp_string[SEEK_STRING_MAX_LENGTH];
    int seek_id = camera_seek_get_seek_id(cam_id);
    sockaddr_un_t server_camera;

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
        mmap(NULL, sizeof(seek_shared_memory_t), PROT_READ, MAP_SHARED, seek_handlers[seek_id].fd_shm, 0);
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

    if (connect(seek_handlers[seek_id].sock, (sockaddr_t *)&server_camera, sizeof(sockaddr_un_t)) < 0) {
        close(seek_handlers[seek_id].sock);
        seek_handlers[seek_id].sock = -1;
        return -1;
    }

    return seek_handlers[seek_id].sock;
}

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

int camera_seek_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size) {
    u_int8_t msg[SEEK_CONFIG_MESSAGE_SIZE];
    int min_size = MIN(buffer_size, sizeof(seek_shared_memory_t));

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

eviewitf_ret_t camera_seek_display(int cam_id) {
    int sock_config;
    sockaddr_un_t server_config;
    u_int8_t msg[SEEK_CONFIG_MESSAGE_SIZE];

    /* Create config socket */;
    sock_config = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_config < 0) {
        return EVIEWITF_FAIL;
    }
    server_config.sun_family = AF_UNIX;
    strcpy(server_config.sun_path, SEEK_SOCKET_CONFIG);

    /* Establish connection */
    if (connect(sock_config, (sockaddr_t *)&server_config, sizeof(sockaddr_un_t)) < 0) {
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

eviewitf_ret_t camera_seek_get_attributes(int device_id, eviewitf_device_attributes_t *attributes) {
    if (device_id > 0) {
        attributes->buffer_size = SEEK_FRAME_WIDTH * SEEK_FRAME_HEIGH * sizeof(float);
        attributes->width = SEEK_FRAME_WIDTH;
        attributes->height = SEEK_FRAME_HEIGH;
        attributes->dt = SEEK_DT;

        return EVIEWITF_OK;
    }
    return EVIEWITF_FAIL;
}

/**
 * \file eviewitf_device.c
 * \brief Common functions for device management
 * \author LACROIX Impulse
 *
 * Common functions for device management (camera, streamer, blender, ...)
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <poll.h>
#include <unistd.h>

#include "eviewitf_priv.h"
#include "mfis_communication.h"

/******************************************************************************************
 * Private definitions
 ******************************************************************************************/
#define MAX_VERSION_SIZE 21

/******************************************************************************************
 * Private structures
 ******************************************************************************************/

/******************************************************************************************
 * Private enumerations
 ******************************************************************************************/

/* Device objects */
static device_object device_objects[EVIEWITF_MAX_DEVICES] = {0};

static int file_descriptors[EVIEWITF_MAX_DEVICES];

/******************************************************************************************
 * Functions
 ******************************************************************************************/

/**
 * \fn int generic_close(int file_descriptor)
 * \brief close device
 *
 * \param file_descriptor: file descriptor on an opened device
 *
 * \return 0 on success otherwise -1
 */
int generic_close(int file_descriptor) {
    /* Close file descriptor */
    return close(file_descriptor);
}

/**
 * \fn int generic_write(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size)
 * \brief write to a device
 *
 * \param file_descriptor: file descriptor on an opened device
 * \param frame_buffer: buffer containing the frame
 * \param buffer_size: size of the frame
 *
 * \return the number of written bytes or -1
 */
int generic_write(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size) {
    /* Write to the device */
    return write(file_descriptor, frame_buffer, buffer_size);
}

/**
 * \fn int device_objects_init()
 * \brief Initialize device_objcets structure
 *
 * \return state of the function. Return 0 if okay
 */
int device_objects_init() {
    int ret = EVIEWITF_OK;
    struct eviewitf_mfis_camera_attributes cameras_attributes[EVIEWITF_MAX_CAMERA + EVIEWITF_MAX_STREAMER] = {0};
    struct eviewitf_mfis_blending_attributes blendings_attributes[EVIEWITF_MAX_BLENDER] = {0};

    /* Get the cameras attributes (including streamers) */
    ret = mfis_get_cam_attributes(cameras_attributes);
    /* Fill device structure */
    if (ret == EVIEWITF_OK) {
        /* Set camera operations */
        for (int i = 0; i < EVIEWITF_MAX_CAMERA + EVIEWITF_MAX_STREAMER; i++) {
            /* File descriptor */
            file_descriptors[i] = -1;
            /* Copy attributes */
            device_objects[i].attributes.buffer_size = cameras_attributes[i].buffer_size;
            device_objects[i].attributes.dt = cameras_attributes[i].dt;
            device_objects[i].attributes.height = cameras_attributes[i].height;
            device_objects[i].attributes.width = cameras_attributes[i].width;
            /* Set operations */
            switch (cameras_attributes[i].cam_type) {
                case EVIEWITF_MFIS_CAM_TYPE_GENERIC:
                    device_objects[i].attributes.type = DEVICE_TYPE_CAMERA;
                    device_objects[i].operations.open = camera_open;
                    device_objects[i].operations.close = generic_close;
                    device_objects[i].operations.write = NULL;
                    device_objects[i].operations.read = camera_read;
                    device_objects[i].operations.display = camera_display;
                    device_objects[i].operations.get_attributes = NULL;
                    break;
                case EVIEWITF_MFIS_CAM_TYPE_VIRTUAL:
                    device_objects[i].attributes.type = DEVICE_TYPE_STREAMER;
                    device_objects[i].operations.open = streamer_open;
                    device_objects[i].operations.close = generic_close;
                    device_objects[i].operations.write = generic_write;
                    device_objects[i].operations.read = NULL;
                    device_objects[i].operations.display = camera_display;
                    device_objects[i].operations.get_attributes = NULL;
                    break;
                case EVIEWITF_MFIS_CAM_TYPE_SEEK:
                    device_objects[i].attributes.type = DEVICE_TYPE_CAMERA_SEEK;
                    device_objects[i].operations.open = camera_seek_open;
                    device_objects[i].operations.close = camera_seek_close;
                    device_objects[i].operations.write = NULL;
                    device_objects[i].operations.read = camera_seek_read;
                    device_objects[i].operations.display = camera_seek_display;
                    device_objects[i].operations.get_attributes = camera_seek_get_attributes;
                    /* Check if there are enough seek instances available */
                    if (camera_seek_register(i) != EVIEWITF_OK) {
                        ret = EVIEWITF_OK;
                    }
                    break;

                default:
                    device_objects[i].attributes.type = DEVICE_TYPE_NONE;
                    device_objects[i].operations.open = NULL;
                    device_objects[i].operations.close = NULL;
                    device_objects[i].operations.write = NULL;
                    device_objects[i].operations.read = NULL;
                    device_objects[i].operations.display = camera_display; /* Force display welcome screen */
                    device_objects[i].operations.get_attributes = NULL;

                    break;
            }
        }
    }

    /* Get the blendings attributes */
    if (ret == EVIEWITF_OK) {
        ret = mfis_get_blend_attributes(blendings_attributes);
    }

    /* Fill device structure */
    if (ret == EVIEWITF_OK) {
        for (int i = 0; i < EVIEWITF_MAX_BLENDER; i++) {
            /* File descriptor */
            file_descriptors[i + EVIEWITF_OFFSET_BLENDER] = -1;
            /* Copy attributes */
            device_objects[i + EVIEWITF_OFFSET_BLENDER].attributes.buffer_size = blendings_attributes[i].buffer_size;
            device_objects[i + EVIEWITF_OFFSET_BLENDER].attributes.dt = blendings_attributes[i].dt;
            device_objects[i + EVIEWITF_OFFSET_BLENDER].attributes.height = blendings_attributes[i].height;
            device_objects[i + EVIEWITF_OFFSET_BLENDER].attributes.width = blendings_attributes[i].width;
            device_objects[i + EVIEWITF_OFFSET_BLENDER].attributes.type = DEVICE_TYPE_BLENDER;
            /* Set operations */
            device_objects[i + EVIEWITF_OFFSET_BLENDER].operations.open = blender_open;
            device_objects[i + EVIEWITF_OFFSET_BLENDER].operations.close = generic_close;
            device_objects[i + EVIEWITF_OFFSET_BLENDER].operations.write = generic_write;
            device_objects[i + EVIEWITF_OFFSET_BLENDER].operations.read = NULL;
            device_objects[i + EVIEWITF_OFFSET_BLENDER].operations.display = NULL;
        }
    }

    return ret;
}

/**
 * \fn get_device_object
 * \brief Get a pointer on the device object
 *
 * \param [in] device_id: device id
 *
 * \return pointer on device object structure
 */
device_object *get_device_object(int device_id) {
    if (device_id < 0 || device_id >= EVIEWITF_MAX_DEVICES) {
        return NULL;
    }
    return &device_objects[device_id];
}

/**
 * \fn int device_open(int device_id)
 * \brief Open a device
 *
 * \param device_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 *
 * \return state of the function. Return 0 if okay
 */
int device_open(int device_id) {
    int ret = EVIEWITF_OK;
    device_object *device;

    /* Test API has been initialized */
    if (eviewitf_is_initialized() == 0) {
        ret = EVIEWITF_NOT_INITIALIZED;
    }

    /* Test already open */
    else if (file_descriptors[device_id] != -1) {
        ret = EVIEWITF_FAIL;
    }

    /* Open device */
    else {
        device = get_device_object(device_id);
        if (device->operations.open == NULL) {
            ret = EVIEWITF_FAIL;
        } else {
            file_descriptors[device_id] = device->operations.open(device_id);
            if (file_descriptors[device_id] == -1) {
                return EVIEWITF_FAIL;
            }
        }
    }

    return ret;
}

/**
 * \fn int device_close(int device_id)
 * \brief Close a device
 *
 * \param device_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 *
 * \return state of the function. Return 0 if okay
 */
int device_close(int device_id) {
    int ret = EVIEWITF_OK;
    device_object *device;

    // Test device has been opened
    if (file_descriptors[device_id] == -1) {
        ret = EVIEWITF_NOT_OPENED;
    }

    else {
        device = get_device_object(device_id);
        if (device->operations.close == NULL) {
            ret = EVIEWITF_FAIL;
        } else {
            if (device->operations.close(file_descriptors[device_id]) != 0) {
                ret = EVIEWITF_FAIL;
            } else {
                file_descriptors[device_id] = -1;
            }
        }
    }

    return ret;
}

/**
 * \fn int device_read(int device_id, uint8_t *frame_buffer, uint32_t buffer_size)
 * \brief Copy frame from physical memory to the given buffer location
 *
 * \param device_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 * \param frame_buffer: buffer to store the incoming frame
 * \param buffer_size: buffer size for coherency check
 * \param offset: offset for seek operation (SEEK_SET)
 *
 * \return state of the function. Return 0 if okay
 */
int device_read(int device_id, uint8_t *frame_buffer, uint32_t buffer_size, off_t offset) {
    int ret = EVIEWITF_OK;
    device_object *device;

    if (frame_buffer == NULL) {
        ret = EVIEWITF_INVALID_PARAM;
    } else if (file_descriptors[device_id] == -1) {
        ret = EVIEWITF_NOT_OPENED;
    }

    else {
        device = get_device_object(device_id);
        if (device->operations.read == NULL) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Seek before read */
            if (lseek(file_descriptors[device_id], offset, SEEK_SET) != offset) {
                ret = EVIEWITF_FAIL;
            } else {
                /* Then read */
                if (device->operations.read(file_descriptors[device_id], frame_buffer, buffer_size) < 0) {
                    ret = EVIEWITF_FAIL;
                }
            }
        }
    }

    return ret;
}

/**
 * \fn eviewitf_blender_write_frame
 * \brief Write a frame to a blender

 * \param device_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 * \param in buffer_size: size of the blender frame buffer
 * \param in buffer: device frame buffer
 * \param offset: offset for seek operation (SEEK_SET
 *
 * \return state of the function. Return 0 if okay
 */
int device_write(int device_id, uint8_t *frame_buffer, uint32_t buffer_size, off_t offset) {
    int ret = EVIEWITF_OK;
    device_object *device;

    if (frame_buffer == NULL) {
        ret = EVIEWITF_INVALID_PARAM;
    }

    else if (file_descriptors[device_id] == -1) {
        ret = EVIEWITF_NOT_OPENED;
    }

    else {
        device = get_device_object(device_id);
        if (device->operations.write == NULL) {
            ret = EVIEWITF_FAIL;
        } else {
            /* Seek before write */
            if (lseek(file_descriptors[device_id], offset, SEEK_SET) != offset) {
                ret = EVIEWITF_FAIL;
            } else {
                /* Then write */
                if (device->operations.write(file_descriptors[device_id], frame_buffer, buffer_size) < 0) {
                    ;
                    ret = EVIEWITF_FAIL;
                }
            }
        }
    }

    return ret;
}

/**
 * \fn int device_poll(int *device_id, int nb_cam, short *event_return)
 * \brief Poll on multiple cameras to check a new frame is available
 *
 * \param device_id: table of device ids to poll between 0 and EVIEWITF_MAX_DEVICES
 *        we assume those values has been tested by the caller
 * \param nb_devices: number of devices on which the polling applies
 * \param ms_timeout: number of millisecond the function should block waiting for a frame, negative value means infinite
 * \param event_return: detected events for each device, 0 if no frame, 1 if a frame is available

 * \return state of the function. Return 0 if okay
 */
int device_poll(int *device_id, int nb_devices, int ms_timeout, short *event_return) {
    struct pollfd pfd[nb_devices];
    int r_poll;
    int ret = EVIEWITF_OK;
    int i;

    if (event_return == NULL) {
        return EVIEWITF_INVALID_PARAM;
    }

    for (i = 0; i < nb_devices; i++) {
        if (ret >= EVIEWITF_OK) {
            if (file_descriptors[device_id[i]] == -1) {
                ret = EVIEWITF_NOT_OPENED;
            }
            pfd[i].fd = file_descriptors[device_id[i]];
            pfd[i].events = POLLIN;
        }
    }

    if (ret >= EVIEWITF_OK) {
        r_poll = poll(pfd, nb_devices, ms_timeout);
        if (r_poll == -1) {
            ret = EVIEWITF_FAIL;
        }
    }

    for (i = 0; i < nb_devices; i++) {
        if (ret >= EVIEWITF_OK) {
            event_return[i] = pfd[i].revents & POLLIN;
        }
    }

    return ret;
}

/**
 * \fn int device_get_attributes(int cam_id)
 * \brief Get device attributes such as buffer size
 *
 * \param device_id: table of device ids to poll between 0 and EVIEWITF_MAX_DEVICES
 *        we assume those values has been tested by the caller
 * \param attributes: pointer on the structure to be filled

 * \return state of the function. Return 0 if okay
 */
int device_get_attributes(int device_id, eviewitf_device_attributes_t *attributes) {
    int ret = EVIEWITF_OK;
    /* Get the device attributes */
    device_object *device = get_device_object(device_id);

    /* Test attributes */
    if (ret >= EVIEWITF_OK) {
        if (attributes == NULL) {
            ret = EVIEWITF_INVALID_PARAM;
        }
    }

    /* Test API has been initialized */
    if (ret >= EVIEWITF_OK) {
        if (eviewitf_is_initialized() == 0) {
            ret = EVIEWITF_NOT_INITIALIZED;
        }
    }

    /* Copy attributes */
    if (ret >= EVIEWITF_OK) {
        device = get_device_object(device_id);
        if (device->operations.get_attributes == NULL) {
            attributes->buffer_size = device->attributes.buffer_size;
            attributes->width = device->attributes.width;
            attributes->height = device->attributes.height;
            attributes->dt = device->attributes.dt;
        } else {
            if (device->operations.get_attributes(device_id, attributes) < 0) {
                ret = EVIEWITF_FAIL;
            }
        }
    }

    return ret;
}
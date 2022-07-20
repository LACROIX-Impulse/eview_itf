/**
 * \file
 * \brief Communication API between A53 and R7 CPUs for camera devices
 * \author LACROIX Impulse
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "eviewitf/eviewitf-camera.h"
#include "eviewitf-priv.h"
#include "cam-ioctl.h"
#include "mfis-communication.h"

/**
 * \fn int camera_open(int cam_id)
 * \brief open a camera device
 *
 * \param cam_id: id of the device between 0 and EVIEWITF_MAX_DEVICES
 *        we assume this value has been tested by the caller
 *
 * \return file descriptor or -1
 */
int camera_open(int cam_id) {
    char device_name[DEVICE_CAMERA_MAX_LENGTH];

    /* Get mfis device filename */
    snprintf(device_name, DEVICE_CAMERA_MAX_LENGTH, DEVICE_CAMERA_NAME, cam_id);
    return open(device_name, O_RDONLY);
}

/**
 * \fn int camera_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size)
 * \brief Read from a camera
 *
 * \param file_descriptor: file descriptor on an opened device
 * \param frame_buffer: buffer containing the frame
 * \param buffer_size: size of the frame
 *
 * \return the number of read bytes or -1
 */
int camera_read(int file_descriptor, uint8_t *frame_buffer, uint32_t buffer_size) {
    /* Read from device */
    return read(file_descriptor, frame_buffer, buffer_size);
}

/**
 * \fn int eviewitf_camera_open(int cam_id)
 * \brief Open a camera device
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_open(int cam_id) {
    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        return EVIEWITF_INVALID_PARAM;
    }

    /* Open device */
    return device_open(cam_id + EVIEWITF_OFFSET_CAMERA);
}

/**
 * \fn int eviewitf_camera_close(int cam_id)
 * \brief Close a camera device
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA

 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_close(int cam_id) {
    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_close(cam_id + EVIEWITF_OFFSET_CAMERA);
}

/**
 * \fn eviewitf_camera_start
 * \brief Request R7 to start camera, currently not exposed in libeviewitf
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_start(int cam_id) {
    int ret = EVIEWITF_OK;
    int param = CAM_STATE_RUNNING;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCSCAMSTATE, &param);
    }
    return ret;
}

/**
 * \fn eviewitf_camera_stop
 * \brief Request R7 to stop camera, currently not exposed in libeviewitf
 *
 * \param cam_id: id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_stop(int cam_id) {
    int ret = EVIEWITF_OK;
    int param = CAM_STATE_SUSPENDED;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCSCAMSTATE, &param);
    }
    return ret;
}

/**
 * \fn int eviewitf_camera_get_frame(int cam_id, uint8_t *frame_buffer, uint32_t buffer_size)
 * \brief Copy frame from physical memory to the given buffer location
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] frame_buffer buffer to store the incoming frame
 * \param[in] buffer_size buffer size for coherency check
 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_get_frame(int cam_id, uint8_t *frame_buffer, uint32_t buffer_size) {
    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        return EVIEWITF_INVALID_PARAM;
    }
    /* Do not check return value for backward compatibility */
    device_seek(cam_id + EVIEWITF_OFFSET_CAMERA, 0, SEEK_SET);

    return device_read(cam_id + EVIEWITF_OFFSET_CAMERA, frame_buffer, buffer_size);
}

/**
 * \fn int eviewitf_camera_get_frame_segment(int cam_id, uint8_t* buffer, uint32_t size, uint32_t offset)
 * \brief Get a copy (from eView context memory) of a segment of the latest frame received from a
 * camera. Segment from offset to offset + buffer_size
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] buffer buffer to store the incoming segment
 * \param[in] size buffer size of the segment
 * \param[in] offset offset of the segment from frame buffer start adress
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The buffer must be allocated by the customer application before to call this function.
 * The size to be allocated for a particular segement can be retrieved from frame metadata and can retrieved through a
 * call to eviewitf_camera_get_frame_metadata.
 */
int eviewitf_camera_get_frame_segment(int cam_id, uint8_t *buffer, uint32_t size, uint32_t offset) {
    int ret = EVIEWITF_OK;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        return EVIEWITF_INVALID_PARAM;
    }

    /* Seek before read */
    ret = device_seek(cam_id + EVIEWITF_OFFSET_CAMERA, offset, SEEK_SET);
    if (ret == EVIEWITF_OK) {
        ret = device_read(cam_id + EVIEWITF_OFFSET_CAMERA, buffer, size);
    }

    return ret;
}

/**
 * \fn int eviewitf_camera_get_frame_metadata(int cam_id, eviewitf_frame_metadata_info_t* frame_metadata)
 * \brief Read frame metadata (which is a frame segment)
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] frame_metadata pointer on metadata structure to be filled
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * The metadata that can be retrieved by this functions are the ones defined in the structure
 eviewitf_frame_metadata_info_t.
 */
int eviewitf_camera_get_frame_metadata(int cam_id, eviewitf_frame_metadata_info_t *frame_metadata) {
    uint32_t offset;
    int ret;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        return EVIEWITF_INVALID_PARAM;
    }

    device_object *device = get_device_object(cam_id + EVIEWITF_OFFSET_CAMERA);
    if (device == NULL) {
        return EVIEWITF_INVALID_PARAM;
    }
    offset = device->attributes.buffer_size - sizeof(eviewitf_frame_metadata_info_t);

    /* Seek before read */
    ret = device_seek(cam_id + EVIEWITF_OFFSET_CAMERA, offset, SEEK_SET);
    if (ret == EVIEWITF_OK) {
        ret = device_read(cam_id + EVIEWITF_OFFSET_CAMERA, (uint8_t *)frame_metadata,
                          sizeof(eviewitf_frame_metadata_info_t));
    }

    return ret;
}

/**
 * \fn int eviewitf_camera_poll(int *cam_id, int nb_cam, int ms_timeout, short *event_return)
 * \brief Poll on multiple cameras to check a new frame is available
 *
 * \param[in] cam_id table of camera ids to poll on (id between 0 and EVIEWITF_MAX_CAMERA)
 * \param[in] nb_cam number of cameras on which the polling applies
 * \param[in] ms_timeout dealy the function should block waiting for a frame, negative value means infinite
 * \param[out] event_return detected events for each camera, 0 if no frame, 1 if a frame is available
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_poll(int *cam_id, int nb_cam, int ms_timeout, short *event_return) {
    if (cam_id == NULL) {
        return EVIEWITF_INVALID_PARAM;
    } else {
        for (int i = 0; i < nb_cam; i++) {
            if ((cam_id[i] < 0) || (cam_id[i] >= EVIEWITF_MAX_CAMERA)) {
                return EVIEWITF_INVALID_PARAM;
            }
        }
    }

    return device_poll(cam_id, nb_cam, ms_timeout, event_return);
}

/**
 * \fn int eviewitf_camera_get_attributes(int cam_id)
 * \brief Get camera attributes such as buffer size
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] attributes pointer on the structure to be filled
 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_get_attributes(int cam_id, eviewitf_device_attributes_t *attributes) {
    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        return EVIEWITF_INVALID_PARAM;
    }

    return device_get_attributes(cam_id + EVIEWITF_OFFSET_CAMERA, attributes);
}

/**
 * \fn int eviewitf_camera_extract_metadata(uint8_t *buf, uint32_t buffer_size,
                              eviewitf_frame_metadata_info_t *frame_metadata)
 * \brief Extract metadata from a frame buffer
 *
 * \param[in] buf pointer on the buffer where the frame is stored
 * \param[in] buffer_size size of the buffer
 * \param[out] frame_metadata pointer on metadata structure to be filled
 * \return state of the function. Return 0 if okay
 */
int eviewitf_camera_extract_metadata(uint8_t *buf, uint32_t buffer_size,
                                     eviewitf_frame_metadata_info_t *frame_metadata) {
    int ret = EVIEWITF_OK;
    uint8_t *ptr_metadata;
    eviewitf_frame_metadata_info_t *metadata = NULL;

    if ((buf == NULL) || (frame_metadata == NULL)) {
        return EVIEWITF_INVALID_PARAM;
    }

    if (buffer_size < sizeof(eviewitf_frame_metadata_info_t)) {
        return EVIEWITF_INVALID_PARAM;
    }

    /* Metadata magic number is located at the end of the buffer if present */
    ptr_metadata = buf + buffer_size - sizeof(eviewitf_frame_metadata_info_t);
    metadata = (eviewitf_frame_metadata_info_t *)ptr_metadata;
    if (metadata->magic_number == FRAME_MAGIC_NUMBER) {
        if (metadata->frame_size > buffer_size) {
            /* Special case where frame's data looks like a magic number */
            ret = EVIEWITF_FAIL;
        } else {
            if ((metadata->frame_width * metadata->frame_height * metadata->frame_bpp) != metadata->frame_size) {
                /* Special case where:                      */
                /* - frame's data looks like a magic number */
                /* - frame_size is lower than buffer_size   */
                ret = EVIEWITF_FAIL;
            } else {
                /* Metadata are present and valid */
                memcpy(frame_metadata, metadata, sizeof(eviewitf_frame_metadata_info_t));
            }
        }
    } else {
        /* Magic number not found, no metadata */
        ret = EVIEWITF_FAIL;
    }

    if (ret != EVIEWITF_OK) {
        /* No metadata available */
        memset(frame_metadata, 0, sizeof(eviewitf_frame_metadata_info_t));
    }

    return ret;
}

/**
 * \fn eviewitf_camera_get_exposure(int cam_id, uint32_t *exposure_us, uint32_t *gain_thou)
 * \brief Get camera's exposure time and gain.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] exposure_us pointer to the returned exposure time in micro seconds
 * \param[out] gain_thou pointer to the returned gain in 1/1000 of unit
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_exposure(int cam_id, uint32_t *exposure_us, uint32_t *gain_thou) {
    int ret;
    struct cam_exp exposure_value;
    if ((exposure_us == NULL) || (gain_thou == NULL)) {
        return EVIEWITF_INVALID_PARAM;
    }

    ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCGCAMEXP, &exposure_value);
    *exposure_us = exposure_value.exp_us;
    *gain_thou = exposure_value.gain_thou;

    return ret;
}

/* clang-format off */
/**
 * \fn eviewitf_camera_get_digital_gains(int cam_id, uint16_t *dg_cf00, uint16_t *dg_cf01, uint16_t *dg_cf10, uint16_t dg_cf11)
 * \brief Get camera's CFA patterns digital gains.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] dg_cf00 pointer to the returned CFA 00 digital gain
 * \param[out] dg_cf01 pointer to the returned CFA 01 digital gain
 * \param[out] dg_cf10 pointer to the returned CFA 10 digital gain
 * \param[out] dg_cf11 pointer to the returned CFA 11 digital gain
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
/* clang-format on */
int eviewitf_camera_get_digital_gains(int cam_id, uint16_t *dg_cf00, uint16_t *dg_cf01, uint16_t *dg_cf10,
                                      uint16_t *dg_cf11) {
    int ret;
    struct cam_dg dg;
    if ((dg_cf00 == NULL) || (dg_cf01 == NULL) || (dg_cf10 == NULL) || (dg_cf11 == NULL)) {
        return EVIEWITF_INVALID_PARAM;
    }

    ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCGCAMDG, &dg);
    *dg_cf00 = dg.cf00;
    *dg_cf01 = dg.cf01;
    *dg_cf10 = dg.cf10;
    *dg_cf11 = dg.cf11;

    return ret;
}

/**
 * \fn eviewitf_camera_get_frame_rate(int cam_id, uint16_t *fps)
 * \brief Get camera's frame rate.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] fps pointer to the returned camera frame rate
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_frame_rate(int cam_id, uint16_t *fps) {
    if (!fps) {
        return EVIEWITF_INVALID_PARAM;
    }
    return mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCGCAMRATE, (void *)fps);
}

/**
 * \fn eviewitf_camera_get_min_exposure(int cam_id, uint32_t *exposure_us, uint32_t *gain_thou)
 * \brief Get camera's minimum exposure time and gain.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] exposure_us pointer to the returned exposure time in micro seconds
 * \param[out] gain_thou pointer to the returned gain in 1/1000 of unit
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_min_exposure(int cam_id, uint32_t *exposure_us, uint32_t *gain_thou) {
    int ret;
    struct cam_exp exposure_value;
    if ((exposure_us == NULL) || (gain_thou == NULL)) {
        return EVIEWITF_INVALID_PARAM;
    }

    ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCGCAMEXPMIN, &exposure_value);
    *exposure_us = exposure_value.exp_us;
    *gain_thou = exposure_value.gain_thou;

    return ret;
}

/**
 * \fn eviewitf_camera_get_max_exposure(int cam_id, uint32_t *exposure_us, uint32_t *gain_thou)
 * \brief Get camera's maximum exposure time and gain.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] exposure_us pointer to the returned exposure time in micro seconds
 * \param[out] gain_thou pointer to the returned gain in 1/1000 of unit
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_max_exposure(int cam_id, uint32_t *exposure_us, uint32_t *gain_thou) {
    int ret;
    struct cam_exp exposure_value;
    if ((exposure_us == NULL) || (gain_thou == NULL)) {
        return EVIEWITF_INVALID_PARAM;
    }

    ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCGCAMEXPMAX, &exposure_value);
    *exposure_us = exposure_value.exp_us;
    *gain_thou = exposure_value.gain_thou;

    return ret;
}

/**
 * \fn  eviewitf_camera_set_exposure(int cam_id, uint32_t exposure_us, uint32_t gain_thou)
 * \brief Set camera's exposure time.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] exposure_us exposure time in micro seconds
 * \param[out] gain_thou gain in 1/1000 of unit
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_exposure(int cam_id, uint32_t exposure_us, uint32_t gain_thou) {
    struct cam_exp exposure_value;
    exposure_value.exp_us = exposure_us;
    exposure_value.gain_thou = gain_thou;
    return mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCSCAMEXP, &exposure_value);
}

/* clang-format off */
/**
 * \fn eviewitf_camera_set_digital_gains(int cam_id, uint16_t dg_cf00, uint16_t dg_cf01, uint16_t dg_cf10, uint16_t dg_cf11)
 * \brief Set camera's CFA patterns digital gains.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] dg_cf00 CFA 00 digital gain
 * \param[out] dg_cf01 CFA 01 digital gain
 * \param[out] dg_cf10 CFA 10 digital gain
 * \param[out] dg_cf11 CFA 11 digital gain
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
/* clang-format on */
int eviewitf_camera_set_digital_gains(int cam_id, uint16_t dg_cf00, uint16_t dg_cf01, uint16_t dg_cf10,
                                      uint16_t dg_cf11) {
    struct cam_dg dg;
    dg.cf00 = dg_cf00;
    dg.cf01 = dg_cf01;
    dg.cf10 = dg_cf10;
    dg.cf11 = dg_cf11;
    return mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCSCAMDG, &dg);
}

/**
 * \fn eviewitf_camera_set_frame_rate(int cam_id, uint16_t fps)
 * \brief Set camera's frame rate.
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] fps camera frame rate
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_frame_rate(int cam_id, uint16_t fps) {
    return mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCSCAMRATE, &fps);
}

/**
 * \fn eviewitf_camera_get_frame_offset(int cam_id, uint32_t *x_offset, uint32_t *y_offset)
 * \brief Get camera's frame offset relative to camera sensor
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] x_offset pointer to the returned frame offset (width)
 * \param[out] y_offset pointer to the returned frame offset (height)
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_frame_offset(int cam_id, uint32_t *x_offset, uint32_t *y_offset) {
    struct cam_pt offset;
    int ret;

    ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCGCAMOFFSET, &offset);
    if (ret < 0) return ret;

    *x_offset = (uint32_t)offset.x;
    *y_offset = (uint32_t)offset.y;
    return 0;
}

/**
 * \fn eviewitf_camera_set_frame_offset(int cam_id, uint32_t x_offset, uint32_t y_offset)
 * \brief Set camera's frame offset relative to camera sensor
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] x_offset frame offset (width)
 * \param[in] y_offset frame offset (height)
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_frame_offset(int cam_id, uint32_t x_offset, uint32_t y_offset) {
    struct cam_pt offset;

    offset.x = (int32_t)x_offset;
    offset.y = (int32_t)y_offset;
    return mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCSCAMOFFSET, &offset);
}

/**
 * \fn eviewitf_camera_get_test_pattern(int cam_id, uint32_t *pattern)
 * \brief Get camera's test pattern
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[out] pattern the current test pattern used
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_test_pattern(int cam_id, uint8_t *pattern) {
    return mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCGCAMTP, pattern);
}

/**
 * \fn int eviewitf_camera_set_test_pattern(int cam_id, uint8_t pattern)
 * \brief Set camera's test pattern
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] pattern test pattern used
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_test_pattern(int cam_id, uint8_t pattern) {
    return mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCSCAMTP, &pattern);
}

/**
 * \fn eviewitf_camera_get_parameter(int cam_id, uint32_t reg_address, uint32_t* reg_value)
 * \brief Get a parameter of a camera.
 * \ingroup cameras
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] reg_address Register address
 * \param[out] reg_value Register Value
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_get_parameter(int cam_id, uint32_t reg_address, uint32_t *reg_value) {
    int ret = EVIEWITF_OK;
    struct cam_reg reg;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else if (reg_value == NULL) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        reg.reg = reg_address;
        ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCGCAMREG, &reg);
        *reg_value = reg.val;
    }

    return ret;
}

/**
 * \fn eviewitf_camera_set_parameter(int cam_id, uint32_t reg_address, uint32_t reg_value)
 * \brief Set a parameter of a camera.
 * \ingroup cameras
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \param[in] reg_address Register address
 * \param[in] reg_value Register Value to set
 * \return return code as specified by the eviewitf_return_code enumeration.
 */
int eviewitf_camera_set_parameter(int cam_id, uint32_t reg_address, uint32_t reg_value) {
    int ret = EVIEWITF_OK;
    struct cam_reg reg;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        reg.reg = reg_address;
        reg.val = reg_value;
        ret = mfis_ioctl_request(MFIS_DEV_CAM, cam_id, IOCSCAMREG, &reg);
    }

    return ret;
}

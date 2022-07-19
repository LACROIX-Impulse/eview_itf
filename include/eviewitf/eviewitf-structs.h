/**
 * \file
 * \brief Header for eViewItf API structures
 * \author LACROIX Impulse
 * \copyright Copyright (c) 2019-2022 LACROIX Impulse
 *
 * Structures used for communication API between A53 and R7 CPUs
 */

#ifndef EVIEWITF_STRUCTS_H
#define EVIEWITF_STRUCTS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \enum eviewitf_return_code
 * \brief Return codes used by eViewItf API
 *        A negative value is therefore an error
 */
typedef enum {
    EVIEWITF_OK,                       /*!< No issues */
    EVIEWITF_BLOCKED = -1,             /*!< eViewItf is blocked in a process / wait for an eView answer */
    EVIEWITF_INVALID_PARAM = -2,       /*!< Bad parameters have been set in a function call */
    EVIEWITF_NOT_INITIALIZED = -3,     /*!< The API is not initialized before a function call */
    EVIEWITF_NOT_OPENED = -4,          /*!< The targeted device is not opened */
    EVIEWITF_FAIL = -5,                /*!< Something has failed during the function call */
    EVIEWITF_ALREADY_INITIALIZED = -6, /*!< The API is already initialized */
} eviewitf_return_code;

/**
 * \struct eviewitf_frame_segment_info_t
 * \brief Structure to hold offset and dt of a frame segment
 */
typedef struct {
    uint32_t offset; /*!< The segments offset (in bytes) */
    uint8_t dt;      /*!< The segment data type */
} __attribute__((packed)) eviewitf_frame_segment_info_t;

/**
 * \struct eviewitf_frame_metadata_info_t
 * \brief Pointers to current camera frame metadata
 *
 * The frames metadata are extra information that can be found at the end of a frame.
 * Note: The frame synchronization flag can be used in the customer application to get a synchronization point between
 * two input cameras. For instance, it can be used to synchronize a first camera with a second camera which has a slower
 * framerate. The frame size can be used to verify the data integrity.
 */
typedef struct {
    uint32_t frame_width;         /*!< The frame width (in pixels) */
    uint32_t frame_height;        /*!< The frame height (in pixels) */
    uint32_t frame_bpp;           /*!< The number of bytes per pixels */
    uint32_t frame_timestamp_lsb; /*!< The timestamp (LSB). */
    uint32_t frame_timestamp_msb; /*!< The timestamp (MSB). */
    uint32_t frame_sync;          /*!< A frame synchronization flag */
    eviewitf_frame_segment_info_t segments[4]
        __attribute__((aligned(4))); /*!< Frame segments offset with a maximum of 4 segments */
    uint32_t reserved[19];           /*!< 32 metadata fields in total. */
    uint32_t frame_size;             /*!< The frame size (in bytes) */
    uint32_t magic_number;           /*!< A memory pattern which marks the end of the metadata (magic number) */
} eviewitf_frame_metadata_info_t;

/**
 * \struct eviewitf_device_attributes_t
 * \brief Structure to get a device (camera, streamer or blender) attributes
 *
 * The device attributes gather some information on a device. The device can be a camera, a streamer or a blender.
 * Note: The data type possible values are conformed to the “MIPI Alliance Specification for CSI-2”.
 * Extra data types can also be added depending on customers’ requests.
 *
 */
typedef struct {
    uint32_t buffer_size; /*!< The buffer size (reception buffer for a camera / writing buffer for a streamer or a
                             blender) */
    uint32_t width;       /*!< The frame width (in pixels) */
    uint32_t height;      /*!< The frame height (in pixels) */
    uint16_t dt;          /*!< The data type (Y only, YUV, RGB…) */
} eviewitf_device_attributes_t;

/**
 * \enum eviewitf_frame_format
 * \brief eViewItf frame format supported to draw bounding boxes.
 */
typedef enum {
    EVIEWITF_FRAME_FORMAT_YUV422SP, /*!< YUV422 semi planar frame format */
} eviewitf_frame_format;

/**
 * \enum eviewitf_bounding_box_display_state
 * \brief  Bounding box feature to be displayed or not.
 */
typedef enum {
    EVIEWITF_BOUNDING_BOX_DISPLAY_DISABLED, /*!< Feature will not be displayed */
    EVIEWITF_BOUNDING_BOX_DISPLAY_ENABLED,  /*!< Feature will be displayed */
} eviewitf_bounding_box_display_state;

/**
 * \enum eviewitf_bounding_box_label
 * \brief eViewItf bounding boxes label supported.
 */
typedef enum {
    EVIEWITF_BOUNDING_BOX_LABEL_PERSON,        /*!< Person bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_BICYCLE,       /*!< Bicycle bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_CAR,           /*!< Car bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_MOTORCYCLE,    /*!< Motorcycle bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_AIRPLANE,      /*!< Airplane bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_BUS,           /*!< Bus bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_TRAIN,         /*!< Train bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_TRUCK,         /*!< Truck bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_BOAT,          /*!< Boat bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_TRAFFIC_LIGHT, /*!< Traffic Light bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_FIRE_HYDRANT,  /*!< Fire hydrant bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_STOP_SIGN,     /*!< Stop sign bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_PARKING_METER, /*!< Parking meter bounding box label */
    EVIEWITF_BOUNDING_BOX_LABEL_UNLABELED,     /*!< Unlabeled bounding box label */
} eviewitf_bounding_box_label;

/**
 * \struct eviewitf_rgb_color_attributes_t
 * \brief Structure to set an RGB color
 *
 */
typedef struct {
    uint8_t red;   /*!< Red channel value */
    uint8_t green; /*!< Green channel value */
    uint8_t blue;  /*!< Blue channel value */
} eviewitf_rgb_color_attributes_t;

/**
 * \struct eviewitf_frame_attributes_t
 * \brief Structure to set a frame attributes
 *
 */
typedef struct {
    uint8_t *buffer;              /*!< Pointer to the frame */
    uint32_t width;               /*!< Frame width */
    uint32_t height;              /*!< Frame height */
    eviewitf_frame_format format; /*!< Frame format*/
} eviewitf_frame_attributes_t;

/**
 * \struct eviewitf_bounding_box_attributes_t
 * \brief Structure to set a bounding box attributes
 *
 */
typedef struct {
    uint32_t x_offset;                               /*!< Bounding box upper right position */
    uint32_t y_offset;                               /*!< Bounding box lower left position */
    uint32_t width;                                  /*!< Bounding box width */
    uint32_t height;                                 /*!< Bounding box height */
    uint8_t line_width;                              /*!< Bounding box line width */
    uint8_t text_size;                               /*!< Bounding box text size */
    eviewitf_rgb_color_attributes_t line_color;      /*!< Bounding box line color */
    eviewitf_rgb_color_attributes_t text_color;      /*!< Bounding box text color */
    eviewitf_bounding_box_label label;               /*!< Bounding box label value */
    uint8_t score;                                   /*!< Bounding box score value (between 0 and 100) */
    eviewitf_bounding_box_display_state label_state; /*!< Bounding box label to be displayed */
    eviewitf_bounding_box_display_state score_state; /*!< Bounding box score to be displayed */
} eviewitf_bounding_box_attributes_t;

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_STRUCTS_H */

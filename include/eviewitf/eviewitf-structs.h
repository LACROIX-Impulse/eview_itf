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
 * \brief Return codes used by eViewItf API
 *        A negative value is therefore an error
 */
typedef enum eviewitf_ret {
    EVIEWITF_OK,                       /*!< No issues */
    EVIEWITF_BLOCKED = -1,             /*!< eViewItf is blocked in a process / wait for an eView answer */
    EVIEWITF_INVALID_PARAM = -2,       /*!< Bad parameters have been set in a function call */
    EVIEWITF_NOT_INITIALIZED = -3,     /*!< The API is not initialized before a function call */
    EVIEWITF_NOT_OPENED = -4,          /*!< The targeted device is not opened */
    EVIEWITF_FAIL = -5,                /*!< Something has failed during the function call */
    EVIEWITF_ALREADY_INITIALIZED = -6, /*!< The API is already initialized */
} eviewitf_ret_t;

/**
 * \brief Structure to hold offset and dt of a frame segment
 */
typedef struct __attribute__((packed)) eviewitf_frame_segment_info {
    uint32_t offset; /*!< The segments offset (in bytes) */
    uint8_t dt;      /*!< The segment data type */
} __attribute__((packed)) eviewitf_frame_segment_info_t;

/**
 * \brief Pointers to current camera frame metadata
 *
 * The frames metadata are extra information that can be found at the end of a frame.
 * Note: The frame synchronization flag can be used in the customer application to get a synchronization point between
 * two input cameras. For instance, it can be used to synchronize a first camera with a second camera which has a slower
 * framerate. The frame size can be used to verify the data integrity.
 */
typedef struct eviewitf_frame_metadata_info {
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
 * \brief Structure to get a device (camera, streamer or blender) attributes
 *
 * The device attributes gather some information on a device. The device can be a camera, a streamer or a blender.
 * Note: The data type possible values are conformed to the “MIPI Alliance Specification for CSI-2”.
 * Extra data types can also be added depending on customers’ requests.
 *
 */
typedef struct eviewitf_device_attributes {
    uint32_t buffer_size; /*!< The buffer size (reception buffer for a camera / writing buffer for a streamer or a
                             blender) */
    uint32_t width;       /*!< The frame width (in pixels) */
    uint32_t height;      /*!< The frame height (in pixels) */
    uint16_t dt;          /*!< The data type (Y only, YUV, RGB…) */
} eviewitf_device_attributes_t;

/**
 * \brief eViewItf frame format supported about plot features.
 */
typedef enum eviewitf_plot_frame_format {
    EVIEWITF_PLOT_FRAME_FORMAT_YUV422SP, /*!< YUV422 semi planar frame format */
    EVIEWITF_PLOT_FRAME_FORMAT_RGB888IL, /*!< RGB888 interleave frame format */
} eviewitf_plot_frame_format_t;

/**
 * \brief  Plot feature to be displayed or not.
 */
typedef enum eviewitf_plot_display_state {
    EVIEWITF_PLOT_DISPLAY_ENABLED,  /*!< Feature will be displayed */
    EVIEWITF_PLOT_DISPLAY_DISABLED, /*!< Feature will not be displayed */
} eviewitf_plot_display_state_t;

/**
 * \brief eViewItf text plot alignment definitions.
 */
typedef enum eviewitf_plot_text_align {
    EVIEWITF_PLOT_TEXT_ALIGN_LEFT,   /*!< Text align left regarding x position */
    EVIEWITF_PLOT_TEXT_ALIGN_CENTER, /*!< Text align center regarding x positions */
    EVIEWITF_PLOT_TEXT_ALIGN_RIGHT,  /*!< Text align right regarding x positions */
} eviewitf_plot_text_align_t;

/**
 * \brief Structure to set an RGB color
 *
 */
typedef struct eviewitf_plot_rgb_color_attributes {
    uint8_t red;   /*!< Red channel value */
    uint8_t green; /*!< Green channel value */
    uint8_t blue;  /*!< Blue channel value */
} eviewitf_plot_rgb_color_attributes_t;

/**
 * \brief Structure to set a frame attributes regarding plot features.
 *
 */
typedef struct eviewitf_plot_frame_attributes {
    uint8_t *buffer;                     /*!< Pointer to the frame */
    uint32_t width;                      /*!< Frame width (in pixels) */
    uint32_t height;                     /*!< Frame height (in pixels) */
    eviewitf_plot_frame_format_t format; /*!< Frame format */
} eviewitf_plot_frame_attributes_t;

/**
 * \brief Structure to set a text to plot attributes.
 *
 */
typedef struct eviewitf_plot_text_attributes {
    eviewitf_plot_rgb_color_attributes_t color; /*!< RGB text color */
    uint32_t x;                                 /*!< Text upper left horizontal position (in pixels) */
    uint32_t y;                                 /*!< Text upper left vertical position (in pixels) */
    char *text;                                 /*!< Text to be plotted */
    uint8_t size;                               /*!< Text size */
    eviewitf_plot_text_align_t alignment;       /*!< Text alignment regarding x position */
} eviewitf_plot_text_attributes_t;

/**
 * \brief Text font size in pixel definition
 */
#define EVIEWITF_PLOT_TEXT_FONT_PIXEL_SIZE (8u)

/**
 * \brief Structure to set a rectangle to plot attributes.
 *
 */
typedef struct eviewitf_plot_rectangle_attributes {
    uint32_t x;                                      /*!< Rectangle upper left horizontal position (in pixels) */
    uint32_t y;                                      /*!< Rectangle upper left vertical position (in pixels) */
    uint32_t width;                                  /*!< Rectangle width (in pixels) */
    uint32_t height;                                 /*!< Rectangle height (in pixels) */
    uint8_t line_width;                              /*!< Rectangle line width (in pixels) */
    eviewitf_plot_rgb_color_attributes_t line_color; /*!< Rectangle line color */
    eviewitf_plot_display_state_t line_state;        /*!< Rectangle line to be displayed */
    eviewitf_plot_rgb_color_attributes_t fill_color; /*!< Rectangle fill color */
    eviewitf_plot_display_state_t fill_state;        /*!< Rectangle to be filled */
} eviewitf_plot_rectangle_attributes_t;

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_STRUCTS_H */

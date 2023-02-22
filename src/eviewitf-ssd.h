/**
 * @file eviewitf-ssd.h
 * @brief Header for SSD relative operations
 * @author LACROIX Impulse
 */

#ifndef SRC_EVIEWITF_SSD_H_
#define SRC_EVIEWITF_SSD_H_

#include <time.h>

#include "eviewitf.h"

/**
 * @typedef timespec_t
 * @brief timespec structure typedef*
 */
typedef struct timespec timespec_t;

/**
 * @fn eviewitf_ret_t eviewitf_ssd_get_output_directory(char **storage_directory)
 * @brief Get SSD output directory
 * @param storage_directory storage directory string pointer
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_ssd_get_output_directory(char **storage_directory);

/**
 * @fn eviewitf_ret_t eviewitf_ssd_record_stream(int camera_id, int duration, char *frames_directory, uint32_t size)
 * @brief Get SSD output directory
 * @param camera_id camera identifier
 * @param duration record duration
 * @param frames_directory frames directory path
 * @param size buffer size to allocate
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_ssd_record_stream(int camera_id, int duration, char *frames_directory, uint32_t size);

/**
 * @fn eviewitf_ret_t eviewitf_ssd_streamer_play(int streamer_id, uint32_t buffer_size, int fps, char *frames_directory)
 * @brief Play a recording on a streamer

 * @param streamer_id: id of the streamer
 * @param buffer_size: size of the streamer buffer
 * @param fps: fps to apply on the recording
 * @param frames_directory: path to the recording
 *
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_ssd_streamer_play(int camera_id, uint32_t buffer_size, int fps, char *frames_directory);

/**
 * @fn eviewitf_ret_t eviewitf_ssd_set_blending(int blender_id, uint32_t buffer_size, char *frame)
 * @brief Set a blending frame from a file
 * @param blender_id: blender identifier
 * @param buffer_size: size of the blending frame
 * @param frame: blending frame file
 * @return return code as specified by the eviewitf_ret_t enumeration.
 */
eviewitf_ret_t eviewitf_ssd_set_blending(int blender_id, uint32_t buffer_size, char *frame);

#endif /* SRC_EVIEWITF_SSD_H_ */

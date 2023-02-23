/**
 * @file eviewitf-video.h
 * @brief Header for eViewItf API regarding video display
 * @author LACROIX Impulse
 * @copyright Copyright (c) 2019-2022 LACROIX Impulse
 * @ingroup video
 *
 * Communication API between A53 and R7 CPUs for video display
 *
 * @addtogroup video
 * @{
 */

#ifndef EVIEWITF_VIDEO_H
#define EVIEWITF_VIDEO_H

#include <stdint.h>
#include "eviewitf-structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Possible video states
 * @{
 */
#define EVIEWITF_VIDEO_STATE_RUNNING   (0x01) /*!< Video is running (play) */
#define EVIEWITF_VIDEO_STATE_SUSPENDED (0x02) /*!< Video is suspended (pause) */
/** @} */

/**
 * @fn eviewitf_ret_t eviewitf_video_resume(int cam_id)
 * @brief Resume video display for a camera device
 *
 * @param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * @return return code as specified by the eviewitf_ret_t enumeration.
 *
 * A camera must be opened before to be able to use it (get_frame, poll, get_parameter, set_parameter).
 * A camera should not be opened by two different process at the same time.
 */
eviewitf_ret_t eviewitf_video_resume(int cam_id);

/**
 * @fn eviewitf_ret_t eviewitf_video_suspend(int cam_id)
 * @brief Suspend video display for a camera device
 *
 * @param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * @return return code as specified by the eviewitf_ret_t enumeration.
 *
 * A camera must be opened before to be able to use it (get_frame, poll, get_parameter, set_parameter).
 * A camera should not be opened by two different process at the same time.
 */
eviewitf_ret_t eviewitf_video_suspend(int cam_id);

/**
 * @fn eviewitf_ret_t eviewitf_video_get_state(int cam_id, uint32_t *state)
 * @brief Gets the video state for a camera device
 *
 * @param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * @param[out] state pointer to the returned camera frame rate
 * @return return code as specified by the eviewitf_ret_t enumeration.
 *
 * A camera must be opened before to be able to use it (get_frame, poll, get_parameter, set_parameter).
 * A camera should not be opened by two different process at the same time.
 */
eviewitf_ret_t eviewitf_video_get_state(int cam_id, uint32_t *state);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_VIDEO_H */

/*! \} */

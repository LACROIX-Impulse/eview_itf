/**
 * \file
 * \brief Header for eViewItf API regarding video display
 * \author LACROIX Impulse
 * \copyright Copyright (c) 2019-2021 LACROIX Impulse
 * \ingroup video
 *
 * Communication API between A53 and R7 CPUs for video display
 *
 * \addtogroup video
 * \{
 */

#ifndef EVIEWITF_VIDEO_H
#define EVIEWITF_VIDEO_H

#include <stdint.h>
#include "eviewitf-structs.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @} */

/**
 * \fn int eviewitf_video_resume(int cam_id)
 * \brief Resume video display for a camera device
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A camera must be opened before to be able to use it (get_frame, poll, get_parameter, set_parameter).
 * A camera should not be opened by two different process at the same time.
 */
int eviewitf_video_resume(int cam_id);

/**
 * \fn int eviewitf_video_suspend(int cam_id)
 * \brief Suspend video display for a camera device
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A camera must already been selected for display
 * A camera should not be opened by two different process at the same time.
 */
int eviewitf_video_suspend(int cam_id);

#ifdef __cplusplus
}
#endif

#endif /* EVIEWITF_VIDEO_H */

/*! \} */

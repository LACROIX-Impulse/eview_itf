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

#include "eviewitf/eviewitf-video.h"
#include "eviewitf-priv.h"
#include "video-ioctl.h"
#include "mfis-communication.h"
#include "cam-ioctl.h"

/**
 * \fn int eviewitf_video_resume(int cam_id)
 * \brief Resume video display for a camera device (act on the R7 VIN IP, not on the sensor nor the serdes)
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A camera must be opened before to be able to use it (get_frame, poll, get_parameter, set_parameter).
 * A camera should not be opened by two different process at the same time.
 */
int eviewitf_video_resume(int cam_id)
{
    int ret = EVIEWITF_OK;
    int param = VIDEO_STATE_RUNNING;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        printf("src/eviewitf-video.c %s(cam_id=%d)\n", __FUNCTION__, cam_id );
        printf("send ioctl MFIS_DEV_VIDEO (%d), cam_id (%d), IOCSVIDSTATE (%lu), param = VIDEO_STATE_RUNNING (%d)\n",
               MFIS_DEV_VIDEO, cam_id, IOCSVIDSTATE, param);
        ret = mfis_ioctl_request(MFIS_DEV_VIDEO, cam_id, IOCSVIDSTATE, &param);
    }
    return ret;
}


/**
 * \fn int eviewitf_video_suspend(int cam_id)
 * \brief Suspend video display for a camera device (act on the R7 VIN IP, not on the sensor nor the serdes)
 *
 * \param[in] cam_id id of the camera between 0 and EVIEWITF_MAX_CAMERA
 * \return return code as specified by the eviewitf_return_code enumeration.
 *
 * A camera must already been selected for display
 * A camera should not be opened by two different process at the same time.
 */
int eviewitf_video_suspend(int cam_id)
{
    int ret = EVIEWITF_OK;
    int param = VIDEO_STATE_SUSPENDED;

    /* Test camera id */
    if ((cam_id < 0) || (cam_id >= EVIEWITF_MAX_CAMERA)) {
        ret = EVIEWITF_INVALID_PARAM;
    } else {
        printf("src/eviewitf-video.c %s(cam_id=%d)\n", __FUNCTION__, cam_id );
        printf("send ioctl MFIS_DEV_VIDEO (%d), cam_id (%d), IOCSVIDSTATE (%d), param = VIDEO_STATE_SUSPENDED (%d)\n",
               MFIS_DEV_VIDEO, cam_id, IOCSVIDSTATE, param);
        ret = mfis_ioctl_request(MFIS_DEV_VIDEO, cam_id, IOCSVIDSTATE, &param);
    }
    return ret;
}

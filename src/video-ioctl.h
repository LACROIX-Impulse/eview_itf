#ifndef _VIDEO_IOCTL_H
#define _VIDEO_IOCTL_H

#include "mfis-ioctl.h"

/**
 * @brief Video states
 */
#define VIDEO_STATE_RUNNING   (0x01) /* Video is running (play) */
#define VIDEO_STATE_SUSPENDED (0x02) /* Video is suspended (pause) */

/**
 * @brief Video I/O operations
 */
#define IOCSVIDSTATE   MFIS_IOW(0, sizeof(uint32_t))        /* Sets the VIN state */


#endif /* _VIDEO_IOCTL_H */

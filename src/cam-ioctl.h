#ifndef _CAM_IOCTL_H
#define _CAM_IOCTL_H

#include "mfis-ioctl.h"

/**
 * @brief Sensor modes
 */
#define CAM_STATE_INACTIVE  (0) /* Camera is inactive */
#define CAM_STATE_READY     (1) /* Camera is configured (not playing) */
#define CAM_STATE_RUNNING   (2) /* Camera is running (play) */
#define CAM_STATE_SUSPENDED (3) /* Camera is suspended (pause) */

/**
 * @brief Sensor readout
 */
#define CAMREADOUT_NONE    (0x00) /* Normal - no operations */
#define CAMREADOUT_VFLIP   (0x01) /* Vertical flip */
#define CAMREADOUT_HMIRROR (0x02) /* Horizontal mirror */

/**
 * @brief Possible test pattern
 */
#define CAMTP_NONE             (0)   /* No test pattern */
#define CAMTP_SOLID_RED        (1)   /* Solid color - red */
#define CAMTP_SOLID_GREEN      (2)   /* Solid color - green */
#define CAMTP_SOLID_BLUE       (3)   /* Solid color - blue */
#define CAMTP_SOLID_VBAR       (4)   /* Vertical bars */
#define CAMTP_SOLID_VBAR_FADED (5)   /* Vertical bars faded */
#define CAMTP_CUSTOM0          (16)  /* Custom pattern */
#define CAMTP_CUSTOM1          (17)  /* Custom pattern */
#define CAMTP_CUSTOM2          (18)  /* Custom pattern */
#define CAMTP_CUSTOM3          (19)  /* Custom pattern */
#define CAMTP_CUSTOM4          (20)  /* Custom pattern */
#define CAMTP_UNKNOWN          (255) /* Unknown test pattern */

/**
 * @brief Sensor registers
 */
struct cam_reg {
    uint32_t reg;
    uint32_t val;
};

/**
 * @brief Sensor gain mask
 */
#define CAMGAINMSK_DIGITAL (0x01)
#define CAMGAINMSK_ANALOG  (0x02)

/**
 * @brief Sensor gains
 */
struct cam_gain {
    uint16_t mask;
    uint16_t analog;
    uint16_t digital;
    uint16_t custom1;
    uint16_t custom2;
    uint16_t custom3;
    uint16_t custom4;
    uint16_t custom5;
};

/**
 * @brief Camera I/O operations
 */
#define IOCGCAMMODE    MFIS_IOR(0, sizeof(uint32_t))        /* Gets the sensor mode */
#define IOCSCAMMODE    MFIS_IOW(1, sizeof(uint32_t))        /* Sets the sensor mode */
#define IOCGCAMEXP     MFIS_IOR(2, sizeof(uint32_t))        /* Gets the sensor exposure (usecs) */
#define IOCSCAMEXP     MFIS_IOW(3, sizeof(uint32_t))        /* Sets the sensor exposure (usecs) */
#define IOCGCAMGAIN    MFIS_IOR(4, sizeof(struct cam_gain)) /* Gets the sensor analog/digital gain */
#define IOCSCAMGAIN    MFIS_IOW(5, sizeof(struct cam_gain)) /* Sets the sensor analog/digital gain */
#define IOCGCAMRATE    MFIS_IOW(6, sizeof(uint8_t))         /* Gets the sensor frame rate */
#define IOCSCAMRATE    MFIS_IOR(7, sizeof(uint8_t))         /* Sets the sensor frame rate */
#define IOCGCAMREADOUT MFIS_IOR(8, sizeof(uint8_t))         /* Gets the sensor image readout */
#define IOCSCAMREADOUT MFIS_IOW(9, sizeof(uint8_t))         /* Sets the sensor image readout */
#define IOCGCAMREG     MFIS_IOW(10, sizeof(struct cam_reg)) /* Sets the sensor image readout */
#define IOCSCAMREG     MFIS_IOW(11, sizeof(struct cam_reg)) /* Sets the sensor image readout */
#define IOCGCAMTEMP    MFIS_IOR(18, sizeof(uint16_t))       /* Gets the sensor temperature */
#define IOCGCAMTP      MFIS_IOR(50, sizeof(uint8_t))        /* Gets the test pattern */
#define IOCSCAMTP      MFIS_IOW(51, sizeof(uint8_t))        /* Sets the test pattern */
#define IOCCAMREBOOT   MFIS_IO(100)                         /* Reboot command */

#endif /* _CAM_IOCTL_H */

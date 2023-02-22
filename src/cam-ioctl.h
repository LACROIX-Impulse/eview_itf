#ifndef _CAM_IOCTL_H
#define _CAM_IOCTL_H

#include "mfis-ioctl.h"

/**
 * @brief Camera states
 */
#define CAM_STATE_INACTIVE  (0x00) /* Camera is inactive */
#define CAM_STATE_READY     (0x01) /* Camera is configured (not playing) */
#define CAM_STATE_RUNNING   (0x02) /* Camera is running (play) */
#define CAM_STATE_SUSPENDED (0x03) /* Camera is suspended (pause) */

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
typedef struct cam_reg {
    uint32_t reg;
    uint32_t val;
} cam_reg_t;

/**
 * @brief Camera point
 */
typedef struct cam_pt {
    int32_t x; /* X axis */
    int32_t y; /* Y axis */
} cam_pt_t;

/**
 * @brief Sensor exposure
 */
typedef struct cam_exp {
    uint32_t exp_us;    /* Exposure duration (usecs) */
    uint32_t gain_thou; /* Gain value 1/1000 */
} cam_exp_t;

/**
 * @brief Sensor CFA digital gains
 */
typedef struct cam_dg {
    uint16_t cf00; /* CFA 00 digital gain */
    uint16_t cf01; /* CFA 01 digital gain */
    uint16_t cf10; /* CFA 10 digital gain */
    uint16_t cf11; /* CFA 11 digital gain */
} cam_dg_t;

/**
 * @brief Camera I/O operations
 */
#define IOCGCAMSTATE   MFIS_IOR(0, sizeof(uint32_t))   /* Gets the sensor state */
#define IOCSCAMSTATE   MFIS_IOW(1, sizeof(uint32_t))   /* Sets the sensor state */
#define IOCGCAMEXP     MFIS_IOR(2, sizeof(cam_exp_t))  /* Gets the sensor exposure parameters */
#define IOCSCAMEXP     MFIS_IOW(3, sizeof(cam_exp_t))  /* Sets the sensor exposure parameters */
#define IOCGCAMEXPMIN  MFIS_IOR(4, sizeof(cam_exp_t))  /* Gets the sensor min exposure parameters */
#define IOCGCAMEXPMAX  MFIS_IOR(5, sizeof(cam_exp_t))  /* Gets the sensor max exposure parameters */
#define IOCGCAMRATE    MFIS_IOR(6, sizeof(uint16_t))   /* Gets the sensor frame rate */
#define IOCSCAMRATE    MFIS_IOW(7, sizeof(uint16_t))   /* Sets the sensor frame rate */
#define IOCGCAMREADOUT MFIS_IOR(8, sizeof(uint8_t))    /* Gets the sensor image readout */
#define IOCSCAMREADOUT MFIS_IOW(9, sizeof(uint8_t))    /* Sets the sensor image readout */
#define IOCGCAMREG     MFIS_IOR(10, sizeof(cam_reg_t)) /* Sets the sensor register */
#define IOCSCAMREG     MFIS_IOW(11, sizeof(cam_reg_t)) /* Sets the sensor register */
#define IOCGCAMTEMP    MFIS_IOR(12, sizeof(uint16_t))  /* Gets the sensor temperature */
#define IOCGCAMOFFSET  MFIS_IOR(13, sizeof(cam_pt_t))  /* Gets the frame offset */
#define IOCSCAMOFFSET  MFIS_IOW(14, sizeof(cam_pt_t))  /* Sets the frame offset */
#define IOCGCAMDG      MFIS_IOR(15, sizeof(cam_dg_t))  /* Gets the sensor CFA digital gains parameters */
#define IOCSCAMDG      MFIS_IOW(16, sizeof(cam_dg_t))  /* Sets the sensor CFA digital gains parameters */
#define IOCGCAMTP      MFIS_IOR(50, sizeof(uint8_t))   /* Gets the test pattern */
#define IOCSCAMTP      MFIS_IOW(51, sizeof(uint8_t))   /* Sets the test pattern */
#define IOCCAMREBOOT   MFIS_IO(100)                    /* Reboot command */

#endif /* _CAM_IOCTL_H */

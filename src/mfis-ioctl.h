#ifndef _MFIS_IOCTL_H
#define _MFIS_IOCTL_H

#include <stdint.h>

/**
 * IOCTL command encoding: 16 bits total
 * - Command number [0:7]
 * - Parameter size in bytes [8:13]
 * - Direction [14:15]
 */
#define MFIS_IOC_NRBITS   8
#define MFIS_IOC_SIZEBITS 6
#define MFIS_IOC_DIRBITS  2

#define MFIS_IOC_NRMASK   ((1 << MFIS_IOC_NRBITS) - 1)
#define MFIS_IOC_SIZEMASK ((1 << MFIS_IOC_SIZEBITS) - 1)
#define MFIS_IOC_DIRMASK  ((1 << MFIS_IOC_DIRBITS) - 1)

#define MFIS_IOC_NRSHIFT   0
#define MFIS_IOC_SIZESHIFT (MFIS_IOC_NRSHIFT + MFIS_IOC_NRBITS)
#define MFIS_IOC_DIRSHIFT  (MFIS_IOC_SIZESHIFT + MFIS_IOC_SIZEBITS)

/**
 * Direction bits, which any architecture can choose to override
 * before including this file.
 */
#define MFIS_IOC_NONE  (0)
#define MFIS_IOC_WRITE (1)
#define MFIS_IOC_READ  (2)

#define MFIS_IOC(dir, nr, size) \
    (((dir) << MFIS_IOC_DIRSHIFT) | ((nr) << MFIS_IOC_NRSHIFT) | ((size) << MFIS_IOC_SIZESHIFT))

/**
 * Used to create numbers
 */
#define MFIS_IO(nr)         MFIS_IOC(MFIS_IOC_NONE, (nr), 0)
#define MFIS_IOR(nr, size)  MFIS_IOC(MFIS_IOC_READ, (nr), size)
#define MFIS_IOW(nr, size)  MFIS_IOC(MFIS_IOC_WRITE, (nr), size)
#define MFIS_IOWR(nr, size) MFIS_IOC(MFIS_IOC_READ | MFIS_IOC_WRITE, (nr), size)

/**
 * Used to decode ioctl numbers
 */
#define MFIS_IOCDIR(cmd) (((cmd) >> MFIS_IOC_DIRSHIFT) & MFIS_IOC_DIRMASK)
#define MFIS_IOCNR(cmd)  (((cmd) >> MFIS_IOC_NRSHIFT) & MFIS_IOC_NRMASK)
#define MFIS_IOCSZ(cmd)  (((cmd) >> MFIS_IOC_SIZESHIFT) & MFIS_IOC_SIZEMASK)

/**
 * I/O device types
 */
#define MFIS_DEV_CAM        (0) /* Camera I/O operations */
#define MFIS_DEV_PIPELINE   (1) /* Pipeline I/O operations */
#define MFIS_DEV_SERIALIZER (2) /* Serializer I/O operations */
#define MFIS_DEV_VIDEO      (3) /* Video I/O operations (VIN only) */

/**
 * @brief MFIS IOCTL header (little endian)
 */
struct mfis_ioctl {
    uint8_t funcid;
    uint8_t devtype;
    uint8_t requester;
    uint8_t devid;
    uint16_t result;
    uint16_t cmd;
    uint32_t arg[0];
};

#endif /* _MFIS_IOCTL_H */

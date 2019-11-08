/**
 * \file mfis_driver_communication.c
 * \brief Handle communication with MFIS kernel driver via IOCTL
 * \author esoftthings
 *
 * Handle the tx and rx of messages from A53 CPU (Linux) to the R7 CPU via the MFIS peripheral.
 *
 */
 
#define _DEFAULT_SOURCE /* Needed to use usleep() function*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/mman.h>
#include "mfis_driver_communication.h"

/******************************************************************************************
* Private definitions
******************************************************************************************/
/* Time between 2 IOCTL read (us) */
#define READ_TIMER  2000
/* Number of IOCTL read before exit */
#define READ_MAX_TRY    50

/* IOCTL definitions */
#define WR_VALUE _IOW ('a', 1, int32_t*)
#define RD_VALUE _IOR ('a', 2, int32_t*)

/******************************************************************************************
* Functions
******************************************************************************************/
/**
 * \fn int mfis_send_request(int32_t *send, int32_t *receive)
 * \brief Send a request to R7 CPU and return its answer.
 *
 * \param send: array of 32bits containing message to send to R7
 * \param receive: array of 32bits that will be filled with R7 answer
 * \return state of the function. Return 0 if okay
 */
int mfis_send_request(int32_t *send, int32_t *receive)
{
    int fd, ret;
    int read_try = 0;
    
    /* Open MFIS IOCTL */
    fd = open("/dev/mfis_device", O_RDWR);
    if(fd < 0)
    {
        fprintf(stderr, "%s() error cannot open ioctl file : %s\n", __FUNCTION__, strerror(errno));
        ret = fd;
        goto out_ret;
    }
    
    /* Send message to MFIS */
    ret = ioctl(fd, WR_VALUE, (int32_t*) send);
    if(ret < 0)
    {
        fprintf(stderr, "%s() ioctl write error : %s\n", __FUNCTION__, strerror(errno));
        goto out_close;
    }
    
    /* Wait for MFIS answer from R7 */
    do{
        ret = ioctl(fd, RD_VALUE, (int32_t*) receive);
        
        /* IOCTL busy, keep waiting for R7 answer */
        if(errno == EAGAIN)
        {
            /* Timeout */
            if(read_try > READ_MAX_TRY)
            {
                fprintf(stderr, "%s() ioctl read timeout : %s\n", __FUNCTION__, strerror(errno));
                goto out_close;
            }

            read_try++;
            usleep(READ_TIMER);
        }
        /* IOCTL error */
        else if(ret < 0)
        {
            fprintf(stderr, "%s() ioctl read error : %s\n", __FUNCTION__, strerror(errno));
            goto out_close;
        }
        
    }while(ret != 0);
    
out_close:
    close(fd);
out_ret:
    return ret;
}

/**
 * \fn uint32_t* mfis_get_virtual_address(const uint32_t physical_address, uint32_t mem_size)
 * \brief Convert memory physical address to virtual one so it can be accessed from userspace (READ ONLY).
 *
 * \param const uint32_t physical_address: physical address to convert
 * \param uint32_t mem_size: size of the memory to convert
 * \return pointer to virtual address (return NULL if error).
 */
uint32_t* mfis_get_virtual_address(const uint32_t physical_address, uint32_t mem_size)
{
    int mem_dev;
    uint32_t* virtual_address = NULL;
     
    
    printf("mfis_get_virtual_address padd=%x size=%d \n", physical_address, mem_size);
    printf("mfis_get_virtual_address padd=%x size=%d \n", physical_address, mem_size);
    printf("mfis_get_virtual_address padd=%x size=%d \n", physical_address, mem_size);
    printf("mfis_get_virtual_address padd=%x size=%d psize=%d \n", physical_address, mem_size, sysconf(_SC_PAGE_SIZE));
    
    mem_dev = open("/dev/mem", O_RDONLY);
    if(mem_dev == -1)
    {
        fprintf(stderr, "%s() error while opening /dev/mem : %s\n", __FUNCTION__, strerror(errno));
        goto out_ret;
    }

    virtual_address = mmap(NULL, mem_size, PROT_READ, MAP_PRIVATE, mem_dev, physical_address);
    if(virtual_address == MAP_FAILED)
    {
        fprintf(stderr, "%s() error MAP_FAILED : %s\n", __FUNCTION__, strerror(errno));
        goto out_close;
    }
    
    printf("mfis_get_virtual_address padd=%x vadd=%x \n", physical_address, virtual_address);

out_close:
     close(mem_dev);
out_ret:
     return virtual_address;
    
}



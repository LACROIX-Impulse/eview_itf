/**
 * \file mfis_api.c
 * \brief Communication API between A53 and R7 CPUs
 * \author esoftthings
 *
 * API to communicate with the R7 CPU from the A53 (Linux).
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "mfis_driver_communication.h"
#include "mfis_api.h"


int main(void) 
{
    int ret=0, idx;
    mfis_api_cam_buffers_t cam_buffers;
    mfis_api_cam_last_buffers_id_t* cam_last_buffers_id = NULL;
    
    printf(" START PROGRAM \n");
    
    memset(&cam_buffers, 0, sizeof(cam_buffers));
    
    ret = mfis_get_cam_buffers(&cam_buffers);
    if(ret != 0)
        printf("main: ERROR cam buffers\n");
        
    cam_last_buffers_id = mfis_get_cam_last_buffers_id();
    if(cam_last_buffers_id == NULL)
        printf("main: ERROR buffer ID \n");
        
    /*printf("cam_buffer id=%x \n",(cam_buffers.cam[0].ptr_buf3[0]));*/
    idx = cam_buffers.cam[0].buffer_size / sizeof(uint32_t);
    idx--;
    printf("cam_buffer size1=%d size2=%d IDX=%d\n", cam_buffers.cam[0].buffer_size, cam_buffers.cam[1].buffer_size, idx);
    printf("cam_buffer buf1=%x, buf2=%x\n", cam_buffers.cam[0].ptr_buf1[0], cam_buffers.cam[0].ptr_buf1[idx]);
    printf("cam_buffer buf1=%x, buf2=%x\n", cam_buffers.cam[0].ptr_buf2[0], cam_buffers.cam[0].ptr_buf2[idx]);
    printf("cam_buffer buf1=%x, buf2=%x\n", cam_buffers.cam[0].ptr_buf3[0], cam_buffers.cam[0].ptr_buf3[idx]);
    
     printf("cam_buffer id=%x \n", cam_last_buffers_id->last_buffer_id[0]);
     printf("cam_buffer id=%x \n", cam_last_buffers_id->last_buffer_id[1]);
     printf("cam_buffer id=%x \n", cam_last_buffers_id->last_buffer_id[2]);
     printf("cam_buffer id=%x \n", cam_last_buffers_id->last_buffer_id[3]);
     printf("cam_buffer id=%x \n", cam_last_buffers_id->last_buffer_id[4]);
     printf("cam_buffer id=%x \n", cam_last_buffers_id->last_buffer_id[5]);
     printf("cam_buffer id=%x \n", cam_last_buffers_id->last_buffer_id[6]);
     printf("cam_buffer id=%x \n", cam_last_buffers_id->last_buffer_id[7]);
     
    
    /*printf("cam_buffer id=%x \n",(cam_buffers->cam[0].last_buffer_id));*/
    
    printf(" END PROGRAM \n");
    return 0;
}



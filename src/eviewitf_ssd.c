/*
 * ssd_write.c
 *
 *  Created on: Nov 29, 2019
 *      Author: root
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <eviewitf.h>
#include <poll.h>
#include <sys/mman.h>
#include <time.h>
#include <sys/ioctl.h>
#include "eviewitf_ssd.h"
#define SSD_MAX_FILENAME_SIZE     512
#define SSD_SIZE_DIR_NAME_PATTERN 7
#define SSD_SIZE_MOUNT_POINT      9
#define RD_VALUE                  _IOR('a', 2, int32_t *)
#define ONE_SEC_NS                1000000000

static const char *SSD_MOUNT_POINT = "/mnt/ssd/";
static const char *SSD_DIR_NAME_PATTERN = "frames_";

int ssd_get_output_directory(char **storage_directory) {
    DIR *dir;
    struct dirent *dirp;
    struct stat statbuf;
    int current_index;
    int next_index = 0;
    int length_index_to_add;
    char filepath[SSD_MAX_FILENAME_SIZE];

    // First try to open ssd mount moint
    if ((dir = opendir(SSD_MOUNT_POINT)) == NULL) {
        return -1;
    }
    while ((dirp = readdir(dir)) != NULL) {
        snprintf(filepath, SSD_MAX_FILENAME_SIZE, "%s%s", SSD_MOUNT_POINT, dirp->d_name);
        lstat(filepath, &statbuf);
        // Look for directories only
        if (S_ISDIR(statbuf.st_mode)) {
            if (strcmp(".", dirp->d_name) == 0                 // Ignore current directory
                || strcmp("..", dirp->d_name) == 0             // Ignore parent directory
                || strcmp("lost+found", dirp->d_name) == 0) {  // Ignore lost+found directory
                continue;
            }
            // Get current index using SSD_SIZE_DIR_NAME_PATTERN
            current_index = atoi(dirp->d_name + SSD_SIZE_DIR_NAME_PATTERN);
            if (current_index >= next_index) {
                next_index = current_index + 1;
            }
        }
    }
    // DIR no more necessary
    closedir(dir);

    length_index_to_add = snprintf(NULL, 0, "%d", next_index);
    *storage_directory = realloc(*storage_directory, sizeof(char) * (SSD_SIZE_MOUNT_POINT + SSD_SIZE_DIR_NAME_PATTERN +
                                                                     length_index_to_add + 1));

    if (*storage_directory == NULL) {
        return -1;
    }
    snprintf(*storage_directory, SSD_SIZE_MOUNT_POINT + SSD_SIZE_DIR_NAME_PATTERN + length_index_to_add + 1, "%s%s%d",
             SSD_MOUNT_POINT, SSD_DIR_NAME_PATTERN, next_index);
    return 0;
}

int ssd_save_camera_stream(int camera_id, int duration, char *frames_directory, uint32_t size) {
    int frame_id = 0;
    int file_ssd = 0;
    char filename_ssd[SSD_MAX_FILENAME_SIZE];
    struct timespec res_start;
    struct timespec res_run;
    struct timespec difft = {0};
    struct stat st;
    char buff_f[size];
    int cam_fd;
    short revents;
    struct pollfd pfd;
    int r_poll;
    // Create frame directory if not existing (and it should not exist)
    if (stat(frames_directory, &st) == -1) {
        mkdir(frames_directory, 0777);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &res_start) != 0) {
        printf("Got an issue with system clock aborting \n");
        return -1;
    }
    res_run = res_start;
    cam_fd = open(mfis_device_filenames[camera_id], O_RDWR);
    pfd.fd = cam_fd;
    pfd.events = POLLIN;
    while (difft.tv_sec < duration) {
        r_poll = poll(&pfd, 1, -1);
        if (r_poll == -1) {
            printf("POLL ERROR \n");
            return -1;
        }
        revents = pfd.revents;
        if (revents & POLLIN) {
            snprintf(filename_ssd, SSD_MAX_FILENAME_SIZE, "%s/%d", frames_directory, frame_id);
            file_ssd = open(filename_ssd, O_CREAT | O_RDWR);
            read(cam_fd, buff_f, size);
            write(file_ssd, buff_f, size);
            close(file_ssd);

            if (clock_gettime(CLOCK_MONOTONIC, &res_run) != 0) {
                printf("Got an issue with system clock aborting \n");
                return -1;
            }

            if ((res_run.tv_nsec - res_start.tv_nsec) < 0) {
                difft.tv_sec = res_run.tv_sec - res_start.tv_sec - 1;
                difft.tv_nsec = 1000000000 + res_run.tv_nsec - res_start.tv_nsec;
            } else {
                difft.tv_sec = res_run.tv_sec - res_start.tv_sec;
                difft.tv_nsec = res_run.tv_nsec - res_start.tv_nsec;
            }
            frame_id++;
        }
    }

    close(cam_fd);
    printf("Time elapsed %ds:%03ld ms, catched %d frames \n", difft.tv_sec, difft.tv_nsec / 100000, frame_id);
    return 0;
}

/**
 * \fn ssd_set_virtual_camera_stream
 * \brief Play a recording on a virtual camera

 * \param in camera_id: id of the camera
 * \param in buffer_size: size of the virtual camera buffer
 * \param in fps: fps to apply on the recording
 * \param in frames_directory: path to the recording
 * \return state of the function. Return 0 if okay
 */
int ssd_set_virtual_camera_stream(int camera_id, uint32_t buffer_size, int fps, char *frames_directory) {
    int ret = EVIEWITF_OK;
    int frame_id = 0;
    int file_ssd = 1;
    char filename_ssd[SSD_MAX_FILENAME_SIZE];
    int cam_fd;
    unsigned long int duration_ns;
    struct timespec res_start;
    struct timespec res_run;
    struct timespec difft = {0};
    char pre_read = 1;
    int test_rw = 0;
    char buff_f[buffer_size];

    /* Test the fps value */
    if (5 > fps) {
        printf("Bad fps value. Please enter a value greater than or equal to 5\n");
        return -1;
    }

    printf("Playing the recording...\n");

    /* The duration between two frames directly depends on the desired FPS */
    duration_ns = ONE_SEC_NS / fps;

    /* Open the virtual camera to write in */
    cam_fd = open(mfis_device_filenames[camera_id], O_WRONLY);
    if (cam_fd == -1) {
        printf("Error opening camera file\n");
        ret = EVIEWITF_FAIL;
    }

    /* First time value */
    if (clock_gettime(CLOCK_MONOTONIC, &res_start) != 0) {
        printf("Got an issue with system clock aborting \n");
        return -1;
    }

    /* Read the frames in the directory */
    while ((-1) != file_ssd) {
        /* Get current time */
        if (clock_gettime(CLOCK_MONOTONIC, &res_run) != 0) {
            printf("Got an issue with system clock aborting \n");
            return -1;
        }

        /* Pre-read the frame during the waiting step */
        if (1 == pre_read) {
            /* Open the file */
            snprintf(filename_ssd, SSD_MAX_FILENAME_SIZE, "%s/%d", frames_directory, frame_id);
            file_ssd = open(filename_ssd, O_RDONLY);

            /* Read the frame from the file (read not tested as a read fail means "end of the loop") */
            read(file_ssd, buff_f, buffer_size);

            /* Close the file */
            close(file_ssd);

            pre_read = 0;
        }

        /* Take the second incrementation into account */
        if ((res_run.tv_nsec - res_start.tv_nsec) < 0) {
            difft.tv_nsec = ONE_SEC_NS + res_run.tv_nsec - res_start.tv_nsec;
        } else {
            difft.tv_nsec = res_run.tv_nsec - res_start.tv_nsec;
        }

        /* Check if the frame should be updated */
        if (difft.tv_nsec >= duration_ns) {
            /* Update the starting time for the duration */
            if (clock_gettime(CLOCK_MONOTONIC, &res_start) != 0) {
                printf("Got an issue with system clock aborting \n");
                return -1;
            }

            /* Write the frame in the virtual camera */
            write(cam_fd, buff_f, buffer_size);
            if ((-1) == test_rw) {
                printf("[Error] Write frame in the virtual camera\n");
                return -1;
            }

            /* Enable the pre-read */
            pre_read = 1;

            /* Update the frame to be read */
            frame_id++;
        }
    }

    close(cam_fd);
    return ret;
}

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
#include <sys/mman.h>
#include <time.h>
#include <sys/ioctl.h>
#include "eviewitf_ssd.h"
#define SSD_MAX_FILENAME_SIZE     512
#define SSD_SIZE_DIR_NAME_PATTERN 7
#define SSD_SIZE_MOUNT_POINT      9
#define RD_VALUE                  _IOR('a', 2, int32_t *)
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
    struct timespec difft;
    struct stat st;
    char buff_f[size];
    int cam_fd;
    uint32_t *addr;
    // Create frame directory if not existing (and it should not exist)
    if (stat(frames_directory, &st) == -1) {
        mkdir(frames_directory, 0777);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &res_start) != 0) {
        printf("Got a issue with system clock aborting \n");
        return -1;
    }
    res_run = res_start;
    cam_fd = open(mfis_device_filenames[camera_id], O_RDWR);

    while (difft.tv_sec < duration) {
        snprintf(filename_ssd, SSD_MAX_FILENAME_SIZE, "%s/%d", frames_directory, frame_id);
        file_ssd = open(filename_ssd, O_CREAT | O_RDWR);
        read(cam_fd, &buff_f, size);
        write(file_ssd, buff_f,size);
        close(file_ssd);
        if (clock_gettime(CLOCK_MONOTONIC, &res_run) != 0) {
            printf("Got a issue with system clock aborting \n");
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
    close(cam_fd);
    printf("Time elapsed %ds:%03ld ms, catched %d frames \n", difft.tv_sec, difft.tv_nsec / 100000, frame_id);
    return 0;
}

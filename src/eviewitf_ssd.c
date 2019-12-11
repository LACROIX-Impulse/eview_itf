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
#include <time.h>
#include "eviewitf_ssd.h"
#define SSD_MAX_FILENAME_SIZE     512
#define SSD_SIZE_DIR_NAME_PATTERN 7
#define SSD_SIZE_MOUNT_POINT      9

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

int ssd_save_camera_stream(int camera_id, int duration, char *frames_directory) {
    int frame_id = 0;
    int file_ssd = 0;
    char filename_ssd[SSD_MAX_FILENAME_SIZE];
    eviewitf_frame_buffer_info_t frame_buffer;
    time_t start_time = time(NULL);
    struct stat st;

    // Create frame directory if not existing (and it should not exist)
    if (stat(frames_directory, &st) == -1) {
        mkdir(frames_directory, 0777);
    }

    while ((time(NULL) - start_time) < duration) {
        snprintf(filename_ssd, SSD_MAX_FILENAME_SIZE, "%s/%d", frames_directory, frame_id);
        file_ssd = open(filename_ssd, O_CREAT | O_RDWR);
        if (file_ssd == -1) {
            printf("Error opening recording file\n");
            return -1;
        }
        if (eviewitf_get_frame(camera_id, &frame_buffer, NULL) < 0) {
            printf("Error getting frame %d\n", frame_id);
            close(file_ssd);
            return -1;
        }
        // Write frame to disk
        write(file_ssd, frame_buffer.ptr_buf, frame_buffer.buffer_size);
        close(file_ssd);
        // Increment frame id
        frame_id++;
    }

    return 0;
}

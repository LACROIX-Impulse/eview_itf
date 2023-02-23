/**
 * @file eviewitf-ssd.c
 * @brief Communication API with SSD
 * @author LACROIX Impulse
 *
 * API to communicate with the SSD
 *
 */

#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <unistd.h>

#include "eviewitf-ssd.h"
#include "eviewitf-priv.h"

#define SSD_MAX_FILENAME_SIZE     512
#define SSD_SIZE_DIR_NAME_PATTERN 7
#define SSD_SIZE_MOUNT_POINT      9
#define ONE_SEC_NS                1000000000

static const char *SSD_MOUNT_POINT = "/mnt/ssd/";
static const char *SSD_DIR_NAME_PATTERN = "frames_";

eviewitf_ret_t eviewitf_ssd_get_output_directory(char **storage_directory) {
    DIR *dir;
    dirent_t *dirp;
    stat_t statbuf;
    int current_index;
    int next_index = 0;
    int length_index_to_add;
    char filepath[SSD_MAX_FILENAME_SIZE];

    // First try to open ssd mount moint
    if ((dir = opendir(SSD_MOUNT_POINT)) == NULL) {
        return EVIEWITF_FAIL;
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
        return EVIEWITF_FAIL;
    }
    snprintf(*storage_directory, SSD_SIZE_MOUNT_POINT + SSD_SIZE_DIR_NAME_PATTERN + length_index_to_add + 1, "%s%s%d",
             SSD_MOUNT_POINT, SSD_DIR_NAME_PATTERN, next_index);
    return EVIEWITF_OK;
}

int eviewitf_ssd_record_stream(int camera_id, int duration, char *frames_directory, uint32_t size) {
    int frame_id = 0;
    int file_ssd = 0;
    char filename_ssd[SSD_MAX_FILENAME_SIZE];
    timespec_t res_start;
    timespec_t res_run;
    timespec_t difft = {0};
    stat_t st;
    char *buff_f;
    short revents;

    // Create frame directory if not existing (and it should not exist)
    if (stat(frames_directory, &st) == -1) {
        mkdir(frames_directory, 0777);
    }
    if (clock_gettime(CLOCK_MONOTONIC, &res_start) != 0) {
        printf("Got an issue with system clock aborting \n");
        return EVIEWITF_FAIL;
    }
    res_run = res_start;
    if (eviewitf_camera_open(camera_id) != EVIEWITF_OK) {
        printf("Error opening device\n");
        return EVIEWITF_FAIL;
    }
    buff_f = malloc(size);
    if (buff_f == NULL) {
        printf("Error Unable to allocate buffer\n");
        eviewitf_camera_close(camera_id);
        return EVIEWITF_FAIL;
    }
    while (difft.tv_sec < duration) {
        if (eviewitf_camera_poll(&camera_id, 1, 2000, &revents) != EVIEWITF_OK) {
            printf("Error polling device\n");
            break;
        }

        if (revents) {
            snprintf(filename_ssd, SSD_MAX_FILENAME_SIZE, "%s/%d", frames_directory, frame_id);
            file_ssd = open(filename_ssd, O_CREAT | O_RDWR, 0666);
            eviewitf_camera_get_frame(camera_id, (uint8_t *)buff_f, size);
            if (write(file_ssd, buff_f, size) != size) {
                printf("Got an issue writing frame on disk\n");
                free(buff_f);
                eviewitf_camera_close(camera_id);
                return EVIEWITF_FAIL;
            }
            close(file_ssd);

            if (clock_gettime(CLOCK_MONOTONIC, &res_run) != 0) {
                printf("Got an issue with system clock aborting \n");
                free(buff_f);
                eviewitf_camera_close(camera_id);
                return EVIEWITF_FAIL;
            }

            if ((res_run.tv_nsec - res_start.tv_nsec) < 0) {
                difft.tv_sec = res_run.tv_sec - res_start.tv_sec - 1;
                difft.tv_nsec = 1000000000 + res_run.tv_nsec - res_start.tv_nsec;
            } else {
                difft.tv_sec = res_run.tv_sec - res_start.tv_sec;
                difft.tv_nsec = res_run.tv_nsec - res_start.tv_nsec;
            }
            frame_id++;
        } else {
            printf("Poll timeout \n");
            break;
        }
    }

    free(buff_f);
    printf("Time elapsed %lds:%03ld ms, catched %d frames \n", difft.tv_sec, difft.tv_nsec / 100000, frame_id);
    if (eviewitf_camera_close(camera_id) != EVIEWITF_OK) {
        printf("Error closing device\n");
        return EVIEWITF_FAIL;
    }
    return EVIEWITF_OK;
}

eviewitf_ret_t eviewitf_ssd_streamer_play(int streamer_id, uint32_t buffer_size, int fps, char *frames_directory) {
    int frame_id = 0;
    int file_ssd = 1;
    char filename_ssd[SSD_MAX_FILENAME_SIZE];
    long int duration_ns;
    timespec_t res_start;
    timespec_t res_run;
    timespec_t difft = {0};
    char pre_read = 1;
    int test_rw = 0;
    uint8_t *buff_f;
    DIR *dir;

    /* Test the fps value */
    if (fps < FPS_MIN_VALUE) {
        printf("Bad fps value. Please enter a value greater than or equal to %d\n", FPS_MIN_VALUE);
        return EVIEWITF_FAIL;
    }
    if (fps > FPS_MAX_VALUE) {
        printf("Bad fps value. Please enter a value lower than or equal to %d\n", FPS_MAX_VALUE);
        return EVIEWITF_FAIL;
    }

    /* Test the frames_directory pointer */
    if (NULL == frames_directory) {
        printf("The recording directory is not set\n");
        return EVIEWITF_FAIL;
    }

    /* Test the directory existence */
    dir = opendir(frames_directory);
    if (NULL == dir) {
        printf("The recording directory cannot be found\n");
        return EVIEWITF_FAIL;
    }

    printf("Playing the recording...\n");

    /* The duration between two frames directly depends on the desired FPS */
    duration_ns = ONE_SEC_NS / fps;

    /* First time value */
    if (clock_gettime(CLOCK_MONOTONIC, &res_start) != 0) {
        printf("Got an issue with system clock aborting \n");
        return EVIEWITF_FAIL;
    }

    if (eviewitf_streamer_open(streamer_id) != EVIEWITF_OK) {
        printf("Error opening device\n");
        return EVIEWITF_FAIL;
    }

    buff_f = malloc(buffer_size);
    if (buff_f == NULL) {
        printf("Error Unable to allocate buffer\n");
        eviewitf_streamer_close(streamer_id);
        return EVIEWITF_FAIL;
    }

    /* Read the frames in the directory */
    while ((-1) != file_ssd) {
        /* Get current time */
        if (clock_gettime(CLOCK_MONOTONIC, &res_run) != 0) {
            printf("Got an issue with system clock aborting \n");
            free(buff_f);
            eviewitf_streamer_close(streamer_id);
            return EVIEWITF_FAIL;
        }

        /* Pre-read the frame during the waiting step */
        if (1 == pre_read) {
            /* Open the file (an open fail means "end of the loop") */
            snprintf(filename_ssd, SSD_MAX_FILENAME_SIZE, "%s/%d", frames_directory, frame_id);
            file_ssd = open(filename_ssd, O_RDONLY);

            if ((-1) != file_ssd) {
                /* Read the frame from the file  */
                test_rw = read(file_ssd, buff_f, buffer_size);
                if ((-1) == test_rw) {
                    printf("[Error] Read frame from the file\n");
                    close(file_ssd);
                    free(buff_f);
                    eviewitf_streamer_close(streamer_id);
                    return EVIEWITF_FAIL;
                }

                /* Close the file */
                close(file_ssd);
            }

            pre_read = 0;
        }

        if ((-1) != file_ssd) {
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
                    free(buff_f);
                    eviewitf_streamer_close(streamer_id);
                    return EVIEWITF_FAIL;
                }

                /* Write the frame in the virtual camera */
                if (EVIEWITF_OK != eviewitf_streamer_write_frame(streamer_id, buff_f, buffer_size)) {
                    printf("[Error] Set a frame in the virtual camera\n");
                    free(buff_f);
                    eviewitf_streamer_close(streamer_id);
                    return EVIEWITF_FAIL;
                }

                /* Enable the pre-read */
                pre_read = 1;

                /* Update the frame to be read */
                frame_id++;
            }
        }
    }

    free(buff_f);
    if (eviewitf_streamer_close(streamer_id) != EVIEWITF_OK) {
        printf("Error closing device\n");
        return EVIEWITF_FAIL;
    }

    return EVIEWITF_OK;
}

eviewitf_ret_t eviewitf_ssd_set_blending(int blender_id, uint32_t buffer_size, char *frame) {
    eviewitf_ret_t ret = EVIEWITF_OK;
    int file_ssd;
    int test_rw = 0;
    uint8_t *buff_f;

    file_ssd = open(frame, O_RDONLY);
    if ((-1) == file_ssd) {
        printf("[Error] Cannot find the input file\n");
        return EVIEWITF_FAIL;
    }

    buff_f = malloc(buffer_size);
    if (buff_f == NULL) {
        printf("[Error] Unable to allocate buffer\n");
        close(file_ssd);
        return EVIEWITF_FAIL;
    }
    /* Read the frame from the file  */
    test_rw = read(file_ssd, buff_f, buffer_size);
    if ((-1) == test_rw) {
        printf("[Error] Read frame from the file\n");
        close(file_ssd);
        free(buff_f);
        return EVIEWITF_FAIL;
    }

    ret = eviewitf_blender_open(blender_id);
    if (ret == EVIEWITF_OK) {
        ret = eviewitf_blender_write_frame(blender_id, buff_f, buffer_size);
    }
    if (ret == EVIEWITF_OK) {
        ret = eviewitf_blender_close(blender_id);
    }

    close(file_ssd);
    free(buff_f);

    return ret;
}

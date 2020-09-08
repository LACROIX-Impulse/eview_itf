#include <eviewitf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "user_processing.h"

int main(void) {
    int ret;
    eviewitf_device_attributes_t camera_attributes;
    uint8_t* frame_buffer;
    int cam_id[1] = {0};
    short event_return[1] = {0};
    user_result_struct result = USER_RESULT_INIT;

    /* Initialize eviewitf */
    ret = eviewitf_init();
    if (ret != EVIEWITF_OK) {
        fprintf(stderr, "Failed to initialize eviewitf\n");
        return -1;
    }

    /* Open camera 0 device */
    ret = eviewitf_camera_open(0);
    if (ret != EVIEWITF_OK) {
        fprintf(stderr, "Failed to open camera 0\n");
        return -1;
    }

    /* Get camera 0 attributes */
    ret = eviewitf_camera_get_attributes(0, &camera_attributes);
    if (ret != EVIEWITF_OK) {
        fprintf(stderr, "Failed to get camera 0 attributes\n");
        return -1;
    }

    /* Allocate the buffer for camera 0 */
    frame_buffer = malloc(camera_attributes.buffer_size);
    if (frame_buffer == NULL) {
        fprintf(stderr, "Failed to allocated buffer 0\n");
        return -1;
    }

    /* Main loop */
    while (user_condition) {
        /* Poll on the camera */
        ret = eviewitf_camera_poll(cam_id, 1, event_return);
        if (ret != EVIEWITF_OK) {
            fprintf(stderr, "Failed to poll on camera\n");
            return -1;
        }

        /* New frame available on camera 0 */
        if (event_return[0] & POLLIN) {
            /* Get frame from camera 0 */
            ret = eviewitf_camera_get_frame(0, frame_buffer, camera_attributes.buffer_size);
            if (ret != EVIEWITF_OK) {
                fprintf(stderr, "Failed to get the frame from camera 0\n");
                return -1;
            }

            /* User processing */
            ret = user_processing(frame_buffer, &result);
            if (ret != USER_OK) {
                fprintf(stderr, "Failed to process the frame\n");
                return -1;
            }
        }
    }

    /* End */
    free(frame_buffer);
    eviewitf_camera_close(0);
    eviewitf_deinit();

    return 0;
}

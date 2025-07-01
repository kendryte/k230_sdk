#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <time.h>

typedef unsigned long tick_t;

#define RT_TOUCH_EVENT_NONE              (0)   /* Touch none */
#define RT_TOUCH_EVENT_UP                (1)   /* Touch up event */
#define RT_TOUCH_EVENT_DOWN              (2)   /* Touch down event */
#define RT_TOUCH_EVENT_MOVE              (3)   /* Touch move event */

struct rt_touch_data
{
    uint8_t  event;                 /* 触摸事件类型 */
    uint8_t  track_id;              /* 触摸点 ID */
    uint8_t  width;                 /* 触摸宽度 */
    uint16_t x_coordinate;          /* X 坐标 */
    uint16_t y_coordinate;          /* Y 坐标 */
    tick_t   timestamp;             /* 时间戳 */
};

const char* touch_event_str(uint8_t event)
{
    switch (event)
    {
    case RT_TOUCH_EVENT_UP: return "UP";
    case RT_TOUCH_EVENT_DOWN: return "DOWN";
    case RT_TOUCH_EVENT_MOVE: return "MOVE";
    default: return "UNKNOWN";
    }
}

int main(int argc, char *argv[]) {
    struct rt_touch_data ev;
    const char *device = "/dev/touch0";

    if (argc > 1) {
        device = argv[1];
    }

    int fd = open(device, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    printf("Listening for touch events on %s...\n", device);

    while (1) {
        ssize_t n = read(fd, &ev, sizeof(ev));
        if (n)
        {
            printf("Touch Event: %-5s | Track ID: %u | X: %u | Y: %u | Width: %u | Timestamp: %lu\n",
            touch_event_str(ev.event),
            ev.track_id,
            ev.x_coordinate,
            ev.y_coordinate,
            ev.width,
            ev.timestamp);

            // 遇到抬起事件退出
            if (ev.event == RT_TOUCH_EVENT_UP) {
                printf("Touch UP detected, exiting.\n");
                break;
            }

        }

    }

    close(fd);
    return 0;
}

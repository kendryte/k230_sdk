#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <DewarpMap.h>

#define WIDTH 1280
#define HEIGHT 720
#define HORIZON (HEIGHT/2)
#define VERTICAL_UP (WIDTH/2)
#define VERTICAL_DOWN (WIDTH/2)

int main(void) {
    uint16_t split_settings[4] = {1, HORIZON, VERTICAL_UP, VERTICAL_DOWN};
    uint32_t map_width = ((WIDTH + (15 & 0b1111)) >> 4) + 2;
    uint32_t map_height = ((HEIGHT + (15 & 0b1111)) >> 4) + 2;
    uint32_t map_size = map_width * map_height * sizeof(uint32_t);
    uint32_t* map = malloc(map_size);
    CreateUpdateWarpPolarMap(
        map, map_width, map_height, 16, 4,
        WIDTH, HEIGHT, WIDTH, HEIGHT,
        WIDTH / 2., HEIGHT / 2., HEIGHT / 2.,
        HORIZON, VERTICAL_UP, VERTICAL_DOWN,
        16, 16, 0x20,
        0, 0, 0, 0,
        60, 60, 60, 60,
        0,
        0, 0, 0, 0,
        0, 0, 0, 0
    );
    fwrite(split_settings, 1, sizeof(split_settings), stdout);
    fwrite(map, 1, map_size, stdout);
    free(map);
    return 0;
}

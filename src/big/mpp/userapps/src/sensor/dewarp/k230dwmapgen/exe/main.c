/**
 * Copyright (c) 2023, Canaan Bright Sight Co., Ltd
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 * 1. Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
 * INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <DewarpMap.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <getopt.h>
#include <yaml.h>

#define COLOR_NONE "\033[0m"
#define RED "\033[1;31;40m"
#define BLUE "\033[1;34;40m"
#define GREEN "\033[1;32;40m"
#define YELLOW "\033[1;33;40m"

#define LOG_LEVEL 3

#define pr_info(fmt, ...) if(LOG_LEVEL>=2)fprintf(stderr,GREEN fmt "\n" COLOR_NONE, ##__VA_ARGS__)
#define pr_warn(fmt, ...) if(LOG_LEVEL>=1)fprintf(stderr,YELLOW fmt "\n" COLOR_NONE, ##__VA_ARGS__)
#define pr_err(fmt, ...) if(LOG_LEVEL>=0)fprintf(stderr,RED fmt "\n" COLOR_NONE, ##__VA_ARGS__)

void usage(const char* argv) {
    fprintf(stderr, "Usage: %s < <YAML file> > <Output binary>\n", argv);
}

#define KEYEQL(x) (strcmp((char*)key_event.data.scalar.value,x)==0)
int parse_value_int(yaml_parser_t* parser) {
    int ret = 0;
    yaml_event_t value_event;
    if (!yaml_parser_parse(parser, &value_event)) {
        return -1;
    }
    ret = atoi((char*) value_event.data.scalar.value);
    yaml_event_delete(&value_event);
    return ret;
}
int parse_data(yaml_parser_t* parser, double* data) {
    yaml_event_t key_event;
    yaml_char_t * ret = NULL;
    unsigned flag_map = 0;
    size_t ptr = 0;
    for(;;) {
        if (!yaml_parser_parse(parser, &key_event)) {
            return -1;
        }
        if (key_event.type == YAML_SCALAR_EVENT) {
            if (!flag_map) {
                return -1;
            }
            if (KEYEQL("data")) {
                // found
                yaml_event_t value_event;
                if (!yaml_parser_parse(parser, &value_event)) {
                    return -1;
                }
                if (value_event.type == YAML_SEQUENCE_START_EVENT) {
                    // TODO: read
                    yaml_event_t data_event;
                    for(;;) {
                        if (!yaml_parser_parse(parser, &data_event)) {
                            return -1;
                        }
                        if (data_event.type == YAML_SCALAR_EVENT) {
                            data[ptr++] = atof((char *)data_event.data.scalar.value);
                        } else if (data_event.type != YAML_SEQUENCE_END_EVENT) {
                            break;
                        }
                        yaml_event_delete(&data_event);
                    }
                    break;
                } else {
                    break;
                }
            }
        } else if (key_event.type == YAML_MAPPING_START_EVENT) {
            flag_map = 1;
        } else if (key_event.type == YAML_STREAM_END_EVENT) {
            yaml_event_delete(&key_event);
            break;
        } 
        yaml_event_delete(&key_event);
    }
    return 0;
}

int parse(
    FILE* file,
    double camera_matrix_data[9],
    double distortion_coefficients[5],
    unsigned* image_width,
    unsigned* image_height
) {
    yaml_parser_t parser;
    yaml_event_t event;

    yaml_parser_initialize(&parser);
    yaml_parser_set_input_file(&parser, file);
    unsigned running = 0xff;
    while (running) {
        if (!yaml_parser_parse(&parser, &event)) {
            break;
        }
        switch (event.type) {
            case YAML_NO_EVENT: break;
            case YAML_STREAM_START_EVENT: break;
            case YAML_MAPPING_START_EVENT: {
                yaml_event_t key_event;
                for(;;) {
                    if (!yaml_parser_parse(&parser, &key_event)) {
                        running = 0;
                        break;
                    }
                    if (key_event.type == YAML_SCALAR_EVENT) {
                        if (KEYEQL("camera_matrix")) {
                            if (parse_data(&parser, camera_matrix_data) < 0) {
                                pr_warn("camera_matrix data not found");
                            }
                        } else if (KEYEQL("distortion_coefficients")) {
                            if (parse_data(&parser, distortion_coefficients) < 0) {
                                pr_warn("distortion_coefficients data not found");
                            }
                        } else if (KEYEQL("image_width")) {
                            *image_width = parse_value_int(&parser);
                        } else if (KEYEQL("image_height")) {
                            *image_height = parse_value_int(&parser);
                        }
                    } else if (key_event.type != YAML_MAPPING_END_EVENT) {
                        yaml_event_delete(&key_event);
                        break;
                    }
                    yaml_event_delete(&key_event);
                }
                break;
            }
            case YAML_STREAM_END_EVENT: {
                running = 0;
                break;
            }
            default: break;
        }
        yaml_event_delete(&event);
    }
    yaml_parser_delete(&parser);

    return running;
}

#define ALIGN_UP(x, align) (((x) + ((align) - 1)) & ~((align)-1))

int main(int argc, char* argv[]) {
    if (argc == 2 && (strcmp(argv[1], "-h") == 0)) {
        usage(argv[0]);
        return 0;
    }
    double camera_matrix_data[9] = {0, 0, 0, 0, 0, 0, 0, 0, 0};
    double distortion_coefficients[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    unsigned image_width = 0, image_height = 0;
    #define bbzero(x) bzero(x,sizeof(x))
    bbzero(camera_matrix_data);
    bbzero(distortion_coefficients);
    if (parse(stdin, camera_matrix_data, distortion_coefficients, &image_width, &image_height)) {
        pr_err("parse error");
    } else {
        pr_info("image_width: %u", image_width);
        pr_info("image_height: %u", image_height);
        pr_info(
            "camera_matrix: %f %f %f %f %f %f %f %f %f",
            camera_matrix_data[0],
            camera_matrix_data[1],
            camera_matrix_data[2],
            camera_matrix_data[3],
            camera_matrix_data[4],
            camera_matrix_data[5],
            camera_matrix_data[6],
            camera_matrix_data[7],
            camera_matrix_data[8]
        );
        pr_info(
            "distortion_coefficients: %f %f %f %f %f",
            distortion_coefficients[0],
            distortion_coefficients[1],
            distortion_coefficients[2],
            distortion_coefficients[3],
            distortion_coefficients[4]
        );
    }
    // FIXME: other mode
    int map_w = (ALIGN_UP(image_width, 16) >> 4) + 1;
    int map_h = (ALIGN_UP(image_height, 16) >> 4) + 1;
    void* map_buffer = calloc(1, map_w * map_h * 4);
    CreateUpdateDewarpMap(
        map_buffer,
        ((image_width + (15 & 0b1111)) >> 4) + 1,
        ((image_height + (15 & 0b1111)) >> 4) + 1,
        16, 4,
        camera_matrix_data,
        distortion_coefficients,
        image_width,
        image_height,
        1., 16, 16
    );
    uint16_t split_settings[4] = {0, 8191, 8191, 8191};
    fwrite(split_settings, 1, sizeof(split_settings), stdout);
    fwrite(map_buffer, 1, map_w * map_h * 4, stdout);
    return 0;
}

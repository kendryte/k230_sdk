/* Copyright (c) 2023, Canaan Bright Sight Co., Ltd
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

/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "demos/lv_demos.h"
#include "examples/lv_examples.h"
#include "lv_port.h"
#include "lvgl.h"
#include <stdlib.h>
#include <unistd.h>

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

typedef void (*demo_func_t)(void);
typedef struct {
    demo_func_t func;
    char *name;
} demo_entry_t;

demo_entry_t demo_list[] = {
    {lv_example_arc_1, "lv_example_arc_1"},
    {lv_example_arc_2, "lv_example_arc_2"},
    {lv_example_animimg_1, "lv_example_animimg_1"},
    {lv_example_bar_1, "lv_example_bar_1"},
    {lv_example_bar_2, "lv_example_bar_2"},
    {lv_example_bar_3, "lv_example_bar_3"},
    {lv_example_bar_4, "lv_example_bar_4"},
    {lv_example_bar_5, "lv_example_bar_5"},
    {lv_example_bar_6, "lv_example_bar_6"},
    {lv_example_btn_1, "lv_example_btn_1"},
    {lv_example_btn_2, "lv_example_btn_2"},
    {lv_example_btn_3, "lv_example_btn_3"},
    {lv_example_btnmatrix_1, "lv_example_btnmatrix_1"},
    {lv_example_btnmatrix_2, "lv_example_btnmatrix_2"},
    {lv_example_btnmatrix_3, "lv_example_btnmatrix_3"},
    {lv_example_calendar_1, "lv_example_calendar_1"},
    {lv_example_canvas_1, "lv_example_canvas_1"},
    {lv_example_canvas_2, "lv_example_canvas_2"},
    {lv_example_chart_1, "lv_example_chart_1"},
    {lv_example_chart_2, "lv_example_chart_2"},
    {lv_example_chart_3, "lv_example_chart_3"},
    {lv_example_chart_4, "lv_example_chart_4"},
    {lv_example_chart_5, "lv_example_chart_5"},
    {lv_example_chart_6, "lv_example_chart_6"},
    {lv_example_chart_7, "lv_example_chart_7"},
    {lv_example_chart_8, "lv_example_chart_8"},
    {lv_example_chart_9, "lv_example_chart_9"},
    {lv_example_checkbox_1, "lv_example_checkbox_1"},
    {lv_example_checkbox_2, "lv_example_checkbox_2"},
    {lv_example_colorwheel_1, "lv_example_colorwheel_1"},
    {lv_example_dropdown_1, "lv_example_dropdown_1"},
    {lv_example_dropdown_2, "lv_example_dropdown_2"},
    {lv_example_dropdown_3, "lv_example_dropdown_3"},
    {lv_example_img_1, "lv_example_img_1"},
    {lv_example_img_2, "lv_example_img_2"},
    {lv_example_img_3, "lv_example_img_3"},
    {lv_example_img_4, "lv_example_img_4"},
    {lv_example_imgbtn_1, "lv_example_imgbtn_1"},
    {lv_example_keyboard_1, "lv_example_keyboard_1"},
    {lv_example_label_1, "lv_example_label_1"},
    {lv_example_label_2, "lv_example_label_2"},
    {lv_example_label_3, "lv_example_label_3"},
    {lv_example_label_4, "lv_example_label_4"},
    {lv_example_label_5, "lv_example_label_5"},
    {lv_example_led_1, "lv_example_led_1"},
    {lv_example_line_1, "lv_example_line_1"},
    {lv_example_list_1, "lv_example_list_1"},
    {lv_example_list_2, "lv_example_list_2"},
    {lv_example_menu_1, "lv_example_menu_1"},
    {lv_example_menu_2, "lv_example_menu_2"},
    {lv_example_menu_3, "lv_example_menu_3"},
    {lv_example_menu_4, "lv_example_menu_4"},
    {lv_example_menu_5, "lv_example_menu_5"},
    {lv_example_meter_1, "lv_example_meter_1"},
    {lv_example_meter_2, "lv_example_meter_2"},
    {lv_example_meter_3, "lv_example_meter_3"},
    {lv_example_meter_4, "lv_example_meter_4"},
    {lv_example_msgbox_1, "lv_example_msgbox_1"},
    {lv_example_obj_1, "lv_example_obj_1"},
    {lv_example_obj_2, "lv_example_obj_2"},
    {lv_example_roller_1, "lv_example_roller_1"},
    {lv_example_roller_2, "lv_example_roller_2"},
    {lv_example_roller_3, "lv_example_roller_3"},
    {lv_example_slider_1, "lv_example_slider_1"},
    {lv_example_slider_2, "lv_example_slider_2"},
    {lv_example_slider_3, "lv_example_slider_3"},
    {lv_example_spinbox_1, "lv_example_spinbox_1"},
    {lv_example_spinner_1, "lv_example_spinner_1"},
    {lv_example_switch_1, "lv_example_switch_1"},
    {lv_example_table_1, "lv_example_table_1"},
    {lv_example_table_2, "lv_example_table_2"},
    {lv_example_tabview_1, "lv_example_tabview_1"},
    {lv_example_tabview_2, "lv_example_tabview_2"},
    {lv_example_textarea_1, "lv_example_textarea_1"},
    {lv_example_textarea_2, "lv_example_textarea_2"},
    {lv_example_textarea_3, "lv_example_textarea_3"},
    {lv_example_tileview_1, "lv_example_tileview_1"},
    {lv_example_win_1, "lv_example_win_1"},
    {lv_example_span_1, "lv_example_span_1"},
    {lv_demo_widgets, "lv_demo_widgets"},
    {lv_demo_benchmark, "lv_demo_benchmark"},
    {lv_demo_stress, "lv_demo_stress"},
    {lv_demo_music, "lv_demo_music"},
};

static void show_help(void)
{
    printf("Usage: ui_demo number\n");
    for (int i = 0; i < sizeof(demo_list) / sizeof(demo_entry_t); i++)
        printf("\t%2d: %s\n", i, demo_list[i].name);
    printf("\tother: %s\n", demo_list[0].name);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        show_help();
        return -1;
    }

    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    uint32_t demo_index = atoi(argv[1]);
    if (demo_index > sizeof(demo_list) / sizeof(demo_entry_t))
        demo_index = 0;
    demo_list[demo_index].func();

    while (1) {
        lv_timer_handler();
        usleep(5 * 1000);
    }

    return 0;
}

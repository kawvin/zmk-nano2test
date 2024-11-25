/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */
#include <zephyr/kernel.h>
#include <zephyr/bluetooth/services/bas.h>
#include <lvgl.h>

// #include "custom_status_screen.h"
#include "central_status.h"
#include <zmk/battery.h>
#include <zmk/display.h>
#include <zmk/display/widgets/battery_status.h>
#include <zmk/events/usb_conn_state_changed.h>
#include <zmk/event_manager.h>
#include <zmk/events/battery_state_changed.h>
#include <zmk/events/ble_active_profile_changed.h>
#include <zmk/events/endpoint_changed.h>
#include <zmk/events/wpm_state_changed.h>
#include <zmk/events/layer_state_changed.h>
#include <zmk/usb.h>
#include <zmk/ble.h>
#include <zmk/endpoints.h>
#include <zmk/keymap.h>
#include <zmk/wpm.h>

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

static lv_color_t c240_image_buffer[240 * 240];
static lv_color_t battery_image_buffer[2][12 * 24];
static lv_color_t bt_image_buffer[12 * 24];
static lv_color_t usb_image_buffer[12 * 24];
static lv_color_t num_image_buffer[16 * 16];
static lv_color_t shift_image_buffer[44 * 24];
static lv_color_t ctrl_image_buffer[56 * 18];
static lv_color_t alt_image_buffer[43 * 24];
static lv_color_t gui_image_buffer[25 * 24];
static lv_color_t cat_image_buffer[204 * 128];

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_DONGLE_BATTERY)
#define SOURCE_OFFSET 1
#else
#define SOURCE_OFFSET 0
#endif

lv_style_t global_style;

LV_IMG_DECLARE(battery1_1_icon);
LV_IMG_DECLARE(battery1_2_icon);
LV_IMG_DECLARE(battery1_3_icon);
LV_IMG_DECLARE(battery1_4_icon);
LV_IMG_DECLARE(battery1_charging_icon);

const lv_img_dsc_t *battery_level[] = {
    &battery1_1_icon, &battery1_2_icon, &battery1_3_icon, &battery1_4_icon, &battery1_charging_icon,
};

LV_IMG_DECLARE(usb_icon);
LV_IMG_DECLARE(bt_icon);
LV_IMG_DECLARE(bt_no_icon);
LV_IMG_DECLARE(num_1_icon);
LV_IMG_DECLARE(num_2_icon);
LV_IMG_DECLARE(num_3_icon);
LV_IMG_DECLARE(num_4_icon);
LV_IMG_DECLARE(num_5_icon);
LV_IMG_DECLARE(num_1_no_icon);
LV_IMG_DECLARE(num_2_no_icon);
LV_IMG_DECLARE(num_3_no_icon);
LV_IMG_DECLARE(num_4_no_icon);
LV_IMG_DECLARE(num_5_no_icon);


LV_IMG_DECLARE(bongo_cat_none);
LV_IMG_DECLARE(bongo_cat_both);
LV_IMG_DECLARE(bongo_cat_left);
LV_IMG_DECLARE(bongo_cat_right);

LV_IMG_DECLARE(ctrl_icon);
LV_IMG_DECLARE(shift_icon);
LV_IMG_DECLARE(alt_icon);
LV_IMG_DECLARE(gui_icon);

// LV_IMG_DECLARE(batterycharge_icon);

const lv_img_dsc_t *sym_num[] = {
    &num_1_icon, &num_2_icon, &num_3_icon, &num_4_icon, &num_5_icon,
};

const lv_img_dsc_t *sym_num_no[] = {
    &num_1_no_icon, &num_2_no_icon, &num_3_no_icon, &num_4_no_icon, &num_5_no_icon,
};


// =======================================================================================
int zmk_widget_status_init(struct zmk_widget_status *widget, lv_obj_t *parent) {

    // widget->obj = lv_obj_create(parent);
    // lv_obj_set_size(widget->obj, 240, 240);
    // lv_obj_t *all = lv_canvas_create(widget->obj);
    // lv_obj_align(all, LV_ALIGN_TOP_LEFT, 0, 0);
    // lv_canvas_set_buffer(all, widget->cbuf, 240, 240, LV_IMG_CF_TRUE_COLOR);

    // sys_slist_append(&widgets, &widget->node);
    // widget_battery_status_init();
    // widget_output_status_init();
    // widget_layer_status_init();
    // widget_modifiers_init();
    // widget_wpm_status_init();

    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);
    lv_draw_rect_dsc_t rect_white_dsc;
    init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);

    lv_obj_t *canvasc240 = lv_canvas_create(lv_scr_act());
    lv_canvas_set_buffer(canvasc240, c240_image_buffer, 240, 240, LV_IMG_CF_TRUE_COLOR);

    // 绘制cat
    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc);
    // x, y是坐标，src是图像的源，可以是文件、结构体指针、Symbol，img_dsc是图像的样式。
    lv_canvas_draw_img(canvasc240, 9, 45, &bongo_cat_none, &img_dsc);

    // 绘制modifier
    // lv_draw_img_dsc_t img_dsc;
    // lv_draw_img_dsc_init(&img_dsc);
    // x,y是坐标，src是图像的源，可以是文件、结构体指针、Symbol，img_dsc是图像的样式。
    lv_canvas_draw_img(canvasc240, 20, 164, &shift_icon, &img_dsc);
    lv_canvas_draw_img(canvasc240, 80, 164, &gui_icon, &img_dsc);
    lv_canvas_draw_img(canvasc240, 120, 167, &ctrl_icon, &img_dsc);
    lv_canvas_draw_img(canvasc240, 182, 164, &alt_icon, &img_dsc);

    // 绘制output
    lv_canvas_draw_img(canvasc240, 72, 208, &num_1_icon, &img_dsc);
    lv_canvas_draw_img(canvasc240, 88, 204, &bt_icon, &img_dsc);
    lv_canvas_draw_img(canvasc240, 102, 204, &usb_icon, &img_dsc);
    lv_canvas_draw_img(canvasc240, 116, 204, &battery1_1_icon, &img_dsc);
    lv_canvas_draw_img(canvasc240, 130, 204, &battery1_charging_icon, &img_dsc);

    // 绘制layer
    lv_draw_label_dsc_t label_dsc;
    // init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_ALIGN_CENTER);
    init_label_dsc(&label_dsc, lv_color_hex(0xFF69B4), &lv_font_montserrat_24,
                   LV_ALIGN_CENTER); // 粉色
    const char *text = "Qwerty";
    // 定义文本位置
    lv_point_t pos = {60, 16}; // 水平居中，垂直居中

    lv_canvas_draw_text(canvasc240, pos.x, pos.y, 120, &label_dsc, text);
    rotate_canvas(canvasc240, c240_image_buffer, 0);
    return 0;
}

lv_obj_t *zmk_widget_status_obj(struct zmk_widget_status *widget) { return widget->obj; }
/*
 *
 * Copyright (c) 2023 The ZMK Contributors
 * SPDX-License-Identifier: MIT
 *
 */

#include <lvgl.h>
#include <zmk/endpoints.h>

#define CANVAS_SIZE 240
#define NUM_SYMBOLS 4 // 修饰键的数量

#define LVGL_BACKGROUND                                                                            \
    IS_ENABLED(CONFIG_NICE_VIEW_WIDGET_INVERTED) ? lv_color_black() : lv_color_white()
#define LVGL_FOREGROUND                                                                            \
    IS_ENABLED(CONFIG_NICE_VIEW_WIDGET_INVERTED) ? lv_color_white() : lv_color_black()

struct status_state {
    /*
    #if IS_ENABLED(CONFIG_ZMK_SPLIT)
        uint8_t battery[ZMK_SPLIT_BLE_PERIPHERAL_COUNT + SOURCE_OFFSET + 1];
        bool usb_present[ZMK_SPLIT_BLE_PERIPHERAL_COUNT + SOURCE_OFFSET + 1];
    #else
        uint8_t battery[ZMK_SPLIT_BLE_PERIPHERAL_COUNT + SOURCE_OFFSET + 1];
        bool usb_present[ZMK_SPLIT_BLE_PERIPHERAL_COUNT + SOURCE_OFFSET + 1];
    #endif
    */

    uint8_t battery[0 + 0 + 1];
    bool usb_present[0 + 0 + 1];
    bool mod_is_active[NUM_SYMBOLS];
    bool charging;

#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    bool usb_is_hid_ready;
    uint8_t layer_index;
    const char *layer_label;
    uint8_t wpm[10];
#else
    bool connected;
#endif
};

struct battery_status_state {
    uint8_t level;
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    bool usb_present;
#endif
};

void rotate_canvas(lv_obj_t *canvas, lv_color_t cbuf[]);
void draw_battery(lv_obj_t *canvas, const struct status_state *state);
void init_label_dsc(lv_draw_label_dsc_t *label_dsc, lv_color_t color, const lv_font_t *font,
                    lv_text_align_t align);
void init_rect_dsc(lv_draw_rect_dsc_t *rect_dsc, lv_color_t bg_color);
void init_line_dsc(lv_draw_line_dsc_t *line_dsc, lv_color_t color, uint8_t width);
void init_arc_dsc(lv_draw_arc_dsc_t *arc_dsc, lv_color_t color, uint8_t width);

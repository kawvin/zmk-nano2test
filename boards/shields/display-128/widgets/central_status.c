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
// LV_IMG_DECLARE(batterycharge_icon);

const lv_img_dsc_t *sym_num[] = {
    &num_1_icon, &num_2_icon, &num_3_icon, &num_4_icon, &num_5_icon,
};

const lv_img_dsc_t *sym_num_no[] = {
    &num_1_no_icon, &num_2_no_icon, &num_3_no_icon, &num_4_no_icon, &num_5_no_icon,
};

static void draw_all(lv_obj_t *widget, lv_color_t cbuf[], const struct status_state *state) {
    lv_obj_t *canvas = lv_obj_get_child(widget, 0);

    lv_draw_label_dsc_t label_dsc;
    // init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_TEXT_ALIGN_RIGHT);
    init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_12, LV_TEXT_ALIGN_RIGHT);
    lv_draw_label_dsc_t label_dsc_wpm;
    init_label_dsc(&label_dsc_wpm, LVGL_FOREGROUND, &lv_font_unscii_8, LV_TEXT_ALIGN_RIGHT);
    lv_draw_rect_dsc_t rect_black_dsc;
    init_rect_dsc(&rect_black_dsc, LVGL_BACKGROUND);
    lv_draw_rect_dsc_t rect_white_dsc;
    init_rect_dsc(&rect_white_dsc, LVGL_FOREGROUND);
    lv_draw_line_dsc_t line_dsc;
    init_line_dsc(&line_dsc, LVGL_FOREGROUND, 1);

    lv_draw_img_dsc_t img_dsc;
    lv_draw_img_dsc_init(&img_dsc); //x,y是坐标，src是图像的源，可以是文件、结构体指针、Symbol，img_dsc是图像的样式。

    // Fill background
    lv_canvas_draw_rect(canvas, 0, 0, CANVAS_SIZE, CANVAS_SIZE, &rect_black_dsc);

// Draw battery---------------------------------
#if IS_ENABLED(CONFIG_ZMK_SPLIT)
    for (int i = 0; i < 1 + ZMK_SPLIT_BLE_PERIPHERAL_COUNT + SOURCE_OFFSET; i++) {
        /*
        if (state->battery[i] > 0 || state->usb_present) {
            lv_obj_clear_flag(symbol, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(symbol, LV_OBJ_FLAG_HIDDEN);
        }
        */
        if (!state->usb_present) {
            if (state->battery[i] > 90) {
                //lv_img_set_src(canvas, battery_level[3]);
                lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[3], &img_dsc);
            } else if (state->battery[i] > 60) {
                lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[2], &img_dsc);
            } else if (state->battery[i] > 30) {
                lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[1], &img_dsc);
            } else {
                lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[0], &img_dsc);
            }
        } else {
            lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[4], &img_dsc);
        }
    }
#else
    for (int i = 0; i < 1 + SOURCE_OFFSET; i++) {
        /*
        if (state->battery[i] > 0 || state->usb_present) {
            lv_obj_clear_flag(symbol, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(symbol, LV_OBJ_FLAG_HIDDEN);
        }
        */
        if (!state->usb_present) {
            if (state->battery[i] > 90) {
                //lv_img_set_src(canvas, battery_level[3]);
                lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[3], &img_dsc);
            } else if (state->battery[i] > 60) {
                lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[2], &img_dsc);
            } else if (state->battery[i] > 30) {
                lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[1], &img_dsc);
            } else {
                lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[0], &img_dsc);
            }
        } else {
            lv_canvas_draw_img(canvas, 116 + (i - 1) * 14, 204, battery_level[4], &img_dsc);
        }
    }
#endif

    // 绘制output---------------------------------

    switch (state->selected_endpoint.transport) {
    case ZMK_TRANSPORT_USB:
        lv_canvas_draw_img(canvas, 102, 204, &usb_icon, &img_dsc);
        // lv_obj_add_flag(modifier_symbols[i]->symbol, LV_OBJ_FLAG_HIDDEN);
        break;
    case ZMK_TRANSPORT_BLE:
        if (state->active_profile_bonded) {
            lv_canvas_draw_img(canvas, 88, 204, &bt_icon, &img_dsc);
            if (state->active_profile_connected) {
                if (state->active_profile_index < (sizeof(sym_num) / sizeof(lv_img_dsc_t *))) {
                    lv_canvas_draw_img(canvas, 72, 208, sym_num[state->active_profile_index],
                                       &img_dsc);
                } else {
                }

            } else {
                if (state->active_profile_index < (sizeof(sym_num) / sizeof(lv_img_dsc_t *))) {
                    lv_canvas_draw_img(canvas, 72, 208, sym_num_no[state->active_profile_index],
                                       &img_dsc);
                } else {
                }
            }
        } else {
            lv_canvas_draw_img(canvas, 88, 204, &bt_no_icon, &img_dsc);
        }
        break;

    // Rotate canvas
    //rotate_canvas(canvas, cbuf);

    /*
     if (state.usb_is_hid_ready) {
            lv_img_set_src(usb_hid_status, &sym_ok);
        } else {
            lv_img_set_src(usb_hid_status, &sym_nok);
        }
        */

    // 绘制layer---------------------------------
    lv_draw_label_dsc_t label_dsc_layer;
    // init_label_dsc(&label_dsc, LVGL_FOREGROUND, &lv_font_montserrat_16, LV_ALIGN_CENTER);
    init_label_dsc(&label_dsc_layer, lv_color_hex(0xFF69B4), &lv_font_montserrat_24,
                   LV_ALIGN_CENTER); //   粉色

    // 定义文本位置
    lv_point_t pos = {60, 16}; // 水平居中，垂直居中

    if (state->layer_label == NULL) {
        char str[10] = {};
        //lv_canvas_draw_text(canvas, pos.x, pos.y, 120, &label_dsc, state->layer_index);
        sprintf(str, "%d", state->layer_index) // Convert integer to string
        lv_canvas_draw_text(canvas, pos.x, pos.y, 120, &label_dsc, str);
    } else {
        lv_canvas_draw_text(canvas, pos.x, pos.y, 120, &label_dsc, state->layer_label);
    }
    // 绘制Bango_cat---------------------------------

    // Rotate canvas
    //  rotate_canvas(canvas, cbuf);
}

// 以下是battery=======================================================================================

struct battery_state {
    uint8_t source;
    uint8_t level;
    bool usb_present;
};

static void set_battery_status(struct zmk_widget_status *widget, struct battery_state state) {

#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
    // widget->state.charging = state.usb_present;
    widget->state.usb_present[state.source] = state.usb_present;
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */

    widget->state.battery[state.source] = state.level;

    draw_all(widget->obj, widget->cbuf, &widget->state);
}

void battery_status_update_cb(struct battery_state state) {
    struct zmk_widget_peripheral_battery_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_battery_status(widget, state); }
}

static struct battery_state peripheral_battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_peripheral_battery_state_changed *ev =
        as_zmk_peripheral_battery_state_changed(eh);
    return (struct battery_state){
        .source = ev->source + SOURCE_OFFSET,
        // .source = ev->source + 1,
        .level = ev->state_of_charge,
        // #if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        //         .usb_present = zmk_usb_is_powered(),
        // #endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

static struct battery_state central_battery_status_get_state(const zmk_event_t *eh) {
    const struct zmk_battery_state_changed *ev = as_zmk_battery_state_changed(eh);
    return (struct battery_state){
        .source = 0,
        .level = (ev != NULL) ? ev->state_of_charge : zmk_battery_state_of_charge(),
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
        .usb_present = zmk_usb_is_powered(),
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
    };
}

static struct battery_state battery_status_get_state(const zmk_event_t *eh) {
    if (as_zmk_peripheral_battery_state_changed(eh) != NULL) {
        return peripheral_battery_status_get_state(eh);
    } else {
        return central_battery_status_get_state(eh);
    }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_peripheral_battery_status, struct battery_state,
                            battery_status_update_cb, battery_status_get_state)
ZMK_SUBSCRIPTION(widget_peripheral_battery_status, zmk_peripheral_battery_state_changed);

#if IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_DONGLE_BATTERY)
#if !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL)

ZMK_SUBSCRIPTION(widget_peripheral_battery_status, zmk_battery_state_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_peripheral_battery_status, zmk_usb_conn_state_changed);
#endif /* IS_ENABLED(CONFIG_USB_DEVICE_STACK) */
#endif /* !IS_ENABLED(CONFIG_ZMK_SPLIT) || IS_ENABLED(CONFIG_ZMK_SPLIT_ROLE_CENTRAL) */
#endif /* IS_ENABLED(CONFIG_ZMK_DONGLE_DISPLAY_DONGLE_BATTERY) */

// 以下是modifier=======================================================================================

struct modifiers_state {
    uint8_t modifiers;
};

struct modifier_symbol {
    uint8_t modifier;
    const lv_img_dsc_t *symbol_dsc;
    lv_obj_t *symbol;
    bool is_active;
};

LV_IMG_DECLARE(ctrl_icon);
struct modifier_symbol ms_control = {
    .modifier = MOD_LCTL | MOD_RCTL,
    .symbol_dsc = &ctrl_icon,
};

LV_IMG_DECLARE(shift_icon);
struct modifier_symbol ms_shift = {
    .modifier = MOD_LSFT | MOD_RSFT,
    .symbol_dsc = &shift_icon,
};

LV_IMG_DECLARE(alt_icon);
struct modifier_symbol ms_alt = {
    .modifier = MOD_LALT | MOD_RALT,
    .symbol_dsc = &alt_icon,
};

LV_IMG_DECLARE(gui_icon);
struct modifier_symbol ms_gui = {
    .modifier = MOD_LGUI | MOD_RGUI,
    .symbol_dsc = &gui_icon,
};

struct modifier_symbol *modifier_symbols[] = {
    // this order determines the order of the symbols
    &ms_gui, &ms_alt, &ms_control, &ms_shift};

#define NUM_SYMBOLS (sizeof(modifier_symbols) / sizeof(struct modifier_symbol *))

static void set_modifiers(struct zmk_widget_status *widget, struct modifiers_state state) {
    for (int i = 0; i < NUM_SYMBOLS; i++) {
        bool mod_is_active = (state.modifiers & modifier_symbols[i]->modifier) > 0;

        if (mod_is_active && !modifier_symbols[i]->is_active) {
            // lv_obj_clear_flag(modifier_symbols[i]->symbol, LV_OBJ_FLAG_HIDDEN);
            modifier_symbols[i]->is_active = true;
        } else if (!mod_is_active && modifier_symbols[i]->is_active) {
            // lv_obj_add_flag(modifier_symbols[i]->symbol, LV_OBJ_FLAG_HIDDEN);
            modifier_symbols[i]->is_active = false;
        }
    }
    draw_all(widget->obj, widget->cbuf, &widget->state);
}

void modifiers_update_cb(struct modifiers_state state) {
    struct zmk_widget_modifiers *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_modifiers(widget, state); }
}

static struct modifiers_state modifiers_get_state(const zmk_event_t *eh) {
    return (struct modifiers_state){.modifiers = zmk_hid_get_explicit_mods()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_modifiers, struct modifiers_state, modifiers_update_cb,
                            modifiers_get_state)

ZMK_SUBSCRIPTION(widget_modifiers, zmk_keycode_state_changed);

// 以下是output=======================================================================================

struct output_status_state {
    struct zmk_endpoint_instance selected_endpoint;
    int active_profile_index;
    bool active_profile_connected;
    bool active_profile_bonded;
    bool usb_is_hid_ready;
};

static void set_output_status(struct zmk_widget_status *widget,
                              const struct output_status_state *state) {

    widget->state.selected_endpoint = state->selected_endpoint;
    widget->state.active_profile_index = state->active_profile_index;
    widget->state.active_profile_connected = state->active_profile_connected;
    widget->state.active_profile_bonded = state->active_profile_bonded;
    widget->state.usb_is_hid_ready = state->usb_is_hid_ready;
    draw_all(widget->obj, widget->cbuf, &widget->state);
}

static void output_status_update_cb(struct output_status_state state) {
    struct zmk_widget_status *widget;
    // SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_output_status(widget->obj, state);
    // }
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_output_status(widget, &state); }
}

static struct output_status_state output_status_get_state(const zmk_event_t *_eh) {
    return (struct output_status_state){.selected_endpoint = zmk_endpoints_selected(),
                                        .active_profile_index = zmk_ble_active_profile_index(),
                                        .active_profile_connected =
                                            zmk_ble_active_profile_is_connected(),
                                        .active_profile_bonded = !zmk_ble_active_profile_is_open(),
                                        .usb_is_hid_ready = zmk_usb_is_hid_ready()};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_output_status, struct output_status_state,
                            output_status_update_cb, output_status_get_state)
ZMK_SUBSCRIPTION(widget_output_status, zmk_endpoint_changed);
#if IS_ENABLED(CONFIG_USB_DEVICE_STACK)
ZMK_SUBSCRIPTION(widget_output_status, zmk_usb_conn_state_changed);
#endif
#if defined(CONFIG_ZMK_BLE)
ZMK_SUBSCRIPTION(widget_output_status, zmk_ble_active_profile_changed);
#endif

// 以下是layer=======================================================================================
static void set_layer_status(struct zmk_widget_status *widget, struct layer_status_state state) {
    widget->state.layer_index = state.index;
    widget->state.layer_label = state.label;

    draw_all(widget->obj, widget->cbuf, &widget->state);
}

static void layer_status_update_cb(struct layer_status_state state) {
    struct zmk_widget_status *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_layer_status(widget, state); }
}

static struct layer_status_state layer_status_get_state(const zmk_event_t *eh) {
    uint8_t index = zmk_keymap_highest_layer_active();
    return (struct layer_status_state){.index = index, .label = zmk_keymap_layer_name(index)};
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_layer_status, struct layer_status_state, layer_status_update_cb,
                            layer_status_get_state)

ZMK_SUBSCRIPTION(widget_layer_status, zmk_layer_state_changed);

// bongo cat =======================================================================================

#define SRC(array) (const void **)array, sizeof(array) / sizeof(lv_img_dsc_t *)

static sys_slist_t widgets = SYS_SLIST_STATIC_INIT(&widgets);

LV_IMG_DECLARE(bongo_cat_right);
LV_IMG_DECLARE(bongo_cat_left);
LV_IMG_DECLARE(bongo_cat_both);
LV_IMG_DECLARE(bongo_cat_none);

#define ANIMATION_SPEED_IDLE 10000
const lv_img_dsc_t *idle_imgs[] = {
    &bongo_cat_left,
    &bongo_cat_right,
    &bongo_cat_none,
};

#define ANIMATION_SPEED_SLOW 2000
const lv_img_dsc_t *slow_imgs[] = {
    &bongo_cat_left, &bongo_cat_left, &bongo_cat_right,
    &bongo_cat_left, &bongo_cat_left, &bongo_cat_right,
};

#define ANIMATION_SPEED_MID 500
const lv_img_dsc_t *mid_imgs[] = {
    &bongo_cat_right,
    &bongo_cat_left,
    &bongo_cat_right,
    &bongo_cat_left,
};

#define ANIMATION_SPEED_FAST 200
const lv_img_dsc_t *fast_imgs[] = {
    &bongo_cat_both,
    &bongo_cat_none,
    &bongo_cat_both,
    &bongo_cat_none,
};

struct bongo_cat_wpm_status_state {
    uint8_t wpm;
};

enum anim_state {
    anim_state_none,
    anim_state_idle,
    anim_state_slow,
    anim_state_mid,
    anim_state_fast
} current_anim_state;

static void set_animation(lv_obj_t *animing, struct bongo_cat_wpm_status_state state) {
    if (state.wpm < 5) {
        if (current_anim_state != anim_state_idle) {
            lv_animimg_set_src(animing, SRC(idle_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_IDLE);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_idle;
        }
    } else if (state.wpm < 30) {
        if (current_anim_state != anim_state_slow) {
            lv_animimg_set_src(animing, SRC(slow_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_SLOW);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_slow;
        }
    } else if (state.wpm < 70) {
        if (current_anim_state != anim_state_mid) {
            lv_animimg_set_src(animing, SRC(mid_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_MID);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_mid;
        }
    } else {
        if (current_anim_state != anim_state_fast) {
            lv_animimg_set_src(animing, SRC(fast_imgs));
            lv_animimg_set_duration(animing, ANIMATION_SPEED_FAST);
            lv_animimg_set_repeat_count(animing, LV_ANIM_REPEAT_INFINITE);
            lv_animimg_start(animing);
            current_anim_state = anim_state_fast;
        }
    }
}

struct bongo_cat_wpm_status_state bongo_cat_wpm_status_get_state(const zmk_event_t *eh) {
    struct zmk_wpm_state_changed *ev = as_zmk_wpm_state_changed(eh);
    return (struct bongo_cat_wpm_status_state){.wpm = ev->state};
};

void bongo_cat_wpm_status_update_cb(struct bongo_cat_wpm_status_state state) {
    struct zmk_widget_bongo_cat *widget;
    SYS_SLIST_FOR_EACH_CONTAINER(&widgets, widget, node) { set_animation(widget->obj, state); }
}

ZMK_DISPLAY_WIDGET_LISTENER(widget_bongo_cat, struct bongo_cat_wpm_status_state,
                            bongo_cat_wpm_status_update_cb, bongo_cat_wpm_status_get_state)

ZMK_SUBSCRIPTION(widget_bongo_cat, zmk_wpm_state_changed);

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
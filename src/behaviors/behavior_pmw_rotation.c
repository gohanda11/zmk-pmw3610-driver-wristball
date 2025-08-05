/*
 * Copyright (c) 2024 The ZMK Contributors
 *
 * SPDX-License-Identifier: MIT
 */

#define DT_DRV_COMPAT zmk_behavior_pmw_rotation

#include <zephyr/device.h>
#include <zephyr/init.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

#include <drivers/behavior.h>
#include <zmk/behavior.h>
#include <zmk/event_manager.h>

LOG_MODULE_DECLARE(zmk, CONFIG_ZMK_LOG_LEVEL);

// PMW3610ドライバーの角度変更関数を外部宣言
extern void pmw3610_set_orientation(uint16_t orientation);

struct behavior_pmw_rotation_config {};

struct behavior_pmw_rotation_data {
    uint16_t current_rotation;
};

static int behavior_pmw_rotation_init(const struct device *dev) {
    struct behavior_pmw_rotation_data *data = dev->data;
    data->current_rotation = 0; // 初期値は0度
    return 0;
}

static int on_keymap_binding_pressed(struct zmk_behavior_binding *binding,
                                     struct zmk_behavior_binding_event event) {
    const struct device *dev = zmk_behavior_get_binding(binding->behavior_dev);
    struct behavior_pmw_rotation_data *data = dev->data;

    // 角度を90度ずつ回転: 0 -> 90 -> 180 -> 270 -> 0
    data->current_rotation = (data->current_rotation + 90) % 360;
    
    // PMW3610ドライバーに新しい角度を設定
    pmw3610_set_orientation(data->current_rotation);
    
    LOG_INF("PMW rotation changed to %d degrees", data->current_rotation);

    return ZMK_BEHAVIOR_OPAQUE;
}

static int on_keymap_binding_released(struct zmk_behavior_binding *binding,
                                      struct zmk_behavior_binding_event event) {
    // キー解放時は何もしない
    return ZMK_BEHAVIOR_OPAQUE;
}

static const struct behavior_driver_api behavior_pmw_rotation_driver_api = {
    .binding_pressed = on_keymap_binding_pressed,
    .binding_released = on_keymap_binding_released,
};


#define DEFINE_PMW_ROTATION(inst) \
    static struct behavior_pmw_rotation_data behavior_pmw_rotation_data_##inst = {}; \
    static const struct behavior_pmw_rotation_config behavior_pmw_rotation_config_##inst = {}; \
    BEHAVIOR_DT_INST_DEFINE(inst, behavior_pmw_rotation_init, NULL, \
        &behavior_pmw_rotation_data_##inst, &behavior_pmw_rotation_config_##inst, \
        POST_KERNEL, CONFIG_KERNEL_INIT_PRIORITY_DEFAULT, \
        &behavior_pmw_rotation_driver_api);

DT_INST_FOREACH_STATUS_OKAY(DEFINE_PMW_ROTATION)
#include <math.h>
#include <string.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#if __has_include("esp_zigbee.h")
#include "esp_zigbee.h"
#else
#include "esp_zigbee_core.h"
#endif
#if __has_include("ha/esp_zigbee_ha_standard.h")
#include "ha/esp_zigbee_ha_standard.h"
#elif __has_include("esp_zigbee_ha_standard.h")
#include "esp_zigbee_ha_standard.h"
#endif

#if !defined(ESP_ZB_BDB_MODE_INITIALIZATION) && defined(EZB_BDB_MODE_INITIALIZATION)
#define ESP_ZB_BDB_MODE_INITIALIZATION EZB_BDB_MODE_INITIALIZATION
#endif

#if !defined(ESP_ZB_BDB_MODE_NETWORK_STEERING) && defined(EZB_BDB_MODE_NETWORK_STEERING)
#define ESP_ZB_BDB_MODE_NETWORK_STEERING EZB_BDB_MODE_NETWORK_STEERING
#endif

#if !defined(ESP_ZB_BDB_MODE_FINDING_BINDING) && defined(EZB_BDB_MODE_FINDING_N_BINDING)
#define ESP_ZB_BDB_MODE_FINDING_BINDING EZB_BDB_MODE_FINDING_N_BINDING
#endif

#ifndef ESP_ZB_BDB_MODE_INITIALIZATION
#define ESP_ZB_BDB_MODE_INITIALIZATION 0
#endif

#ifndef ESP_ZB_BDB_MODE_NETWORK_STEERING
#define ESP_ZB_BDB_MODE_NETWORK_STEERING 0
#endif

#ifndef ESP_ZB_BDB_MODE_FINDING_BINDING
#define ESP_ZB_BDB_MODE_FINDING_BINDING ESP_ZB_BDB_MODE_NETWORK_STEERING
#endif

#if (defined(EZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) || defined(EZB_ZCL_CLUSTER_ID_COLOR_CONTROL))
#define esp_zb_bdb_start_top_level_commissioning ezb_bdb_start_top_level_commissioning
#define esp_zb_bdb_is_factory_new ezb_bdb_is_factory_new
#endif
#include "led_strip.h"
#include "nvs_flash.h"


#if (defined(EZB_ZCL_ATTR_ON_OFF_ON_OFF_ID) || defined(EZB_ZCL_CLUSTER_ID_COLOR_CONTROL))
#define esp_zb_zcl_set_attr_value_message_t ezb_zcl_set_attr_value_message_t
#define esp_zb_app_signal_t ezb_app_signal_t
#define esp_zb_callback_t ezb_callback_t
#define esp_zb_ieee_addr_t ezb_ieee_addr_t
#define esp_zb_platform_config_t ezb_platform_config_t
#define esp_zb_cfg_t ezb_cfg_t
#define esp_zb_color_dimmable_light_cfg_t ezb_color_dimmable_light_cfg_t
#define esp_zb_ep_list_t ezb_ep_list_t
#define esp_zb_cluster_list_t ezb_cluster_list_t
#define esp_zb_endpoint_config_t ezb_endpoint_config_t

#define ESP_ZB_DEFAULT_RADIO_CONFIG EZB_DEFAULT_RADIO_CONFIG
#define ESP_ZB_DEFAULT_HOST_CONFIG EZB_DEFAULT_HOST_CONFIG
#define ESP_ZB_ZED_CONFIG EZB_ZED_CONFIG
#define ESP_ZB_DEFAULT_COLOR_DIMMABLE_LIGHT_CONFIG EZB_DEFAULT_COLOR_DIMMABLE_LIGHT_CONFIG
#define ESP_ZB_AF_HA_PROFILE_ID EZB_AF_HA_PROFILE_ID
#define ESP_ZB_HA_COLOR_DIMMABLE_LIGHT_DEVICE_ID EZB_HA_COLOR_DIMMABLE_LIGHT_DEVICE_ID

#define ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP EZB_ZDO_SIGNAL_SKIP_STARTUP
#define ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START EZB_BDB_SIGNAL_DEVICE_FIRST_START
#define ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT EZB_BDB_SIGNAL_DEVICE_REBOOT
#define ESP_ZB_BDB_SIGNAL_STEERING EZB_BDB_SIGNAL_STEERING

#define ESP_ZB_ZCL_CLUSTER_ID_ON_OFF EZB_ZCL_CLUSTER_ID_ON_OFF
#define ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL EZB_ZCL_CLUSTER_ID_LEVEL_CONTROL
#define ESP_ZB_ZCL_CLUSTER_ID_COLOR_CONTROL EZB_ZCL_CLUSTER_ID_COLOR_CONTROL
#define ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID EZB_ZCL_ATTR_ON_OFF_ON_OFF_ID
#define ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID EZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID
#define ESP_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_X_ID EZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_X_ID
#define ESP_ZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_Y_ID EZB_ZCL_ATTR_COLOR_CONTROL_CURRENT_Y_ID
#define ESP_ZB_ZCL_ATTR_TYPE_BOOL EZB_ZCL_ATTR_TYPE_BOOL
#define ESP_ZB_ZCL_ATTR_TYPE_U8 EZB_ZCL_ATTR_TYPE_U8
#define ESP_ZB_ZCL_ATTR_TYPE_U16 EZB_ZCL_ATTR_TYPE_U16
#define ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID EZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID
#define ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID EZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID

#define esp_zb_scheduler_alarm ezb_scheduler_alarm
#define esp_zb_get_long_address ezb_get_long_address
#define esp_zb_platform_config ezb_platform_config
#define esp_zb_init ezb_init
#define esp_zb_ep_list_create ezb_ep_list_create
#define esp_zb_color_dimmable_light_clusters_create ezb_color_dimmable_light_clusters_create
#define esp_zb_basic_cluster_add_attr ezb_basic_cluster_add_attr
#define esp_zb_ep_list_add_ep ezb_ep_list_add_ep
#define esp_zb_device_register ezb_device_register
#define esp_zb_zcl_register_set_attr_value_cb ezb_zcl_register_set_attr_value_cb
#define esp_zb_start ezb_start
#define esp_zb_main_loop_iteration ezb_main_loop_iteration
#endif

static const char *TAG = "zha_rgb_light";

#ifndef FIRMWARE_VERSION
#define FIRMWARE_VERSION "dev"
#endif

#define HA_ESP_LIGHT_ENDPOINT 10
#define INSTALL_CODE_POLICY_ENABLE false

/* ESP32-C6 SuperMini встроенный WS2812 RGB LED */
#define RGB_LED_GPIO GPIO_NUM_8
#define RGB_LED_NUM_PIXELS 1
#define RGB_LED_RMT_RES_HZ (10 * 1000 * 1000)
#define PAIR_BUTTON_GPIO GPIO_NUM_9
#define BUTTON_DEBOUNCE_US (30 * 1000)
#define BUTTON_DOUBLE_CLICK_WINDOW_US (400 * 1000)

typedef struct {
    bool on;
    uint8_t level;      /* Zigbee level 0..254 */
    uint16_t x;         /* Zigbee color X (0..65535) */
    uint16_t y;         /* Zigbee color Y (0..65535) */
    uint8_t r;
    uint8_t g;
    uint8_t b;
} rgb_light_state_t;

static rgb_light_state_t s_light = {
    .on = false,
    .level = 254,
    .x = 30140, /* ~= 0.46 */
    .y = 32768, /* ~= 0.50 */
    .r = 255,
    .g = 180,
    .b = 120,
};

static led_strip_handle_t s_led_strip = NULL;
static bool s_button_state = true;
static bool s_button_raw_state = true;
static int64_t s_button_last_change_us = 0;
static int s_button_click_count = 0;
static int64_t s_button_last_press_us = 0;

static esp_err_t pair_button_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PAIR_BUTTON_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t err = gpio_config(&io_conf);
    if (err == ESP_OK) {
        s_button_state = gpio_get_level(PAIR_BUTTON_GPIO);
        s_button_raw_state = s_button_state;
        s_button_last_change_us = esp_timer_get_time();
    }
    return err;
}

static esp_err_t rgb_led_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = RGB_LED_GPIO,
        .max_leds = RGB_LED_NUM_PIXELS,
        .led_model = LED_MODEL_WS2812,
        .color_component_format = LED_STRIP_COLOR_COMPONENT_FMT_GRB,
        .flags = {
            .invert_out = false,
        },
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = RGB_LED_RMT_RES_HZ,
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false,
        },
    };

    esp_err_t err = led_strip_new_rmt_device(&strip_config, &rmt_config, &s_led_strip);
    if (err != ESP_OK) {
        return err;
    }

    led_strip_clear(s_led_strip);
    return ESP_OK;
}

static uint8_t clamp_u8(float v)
{
    if (v < 0.0f) {
        return 0;
    }
    if (v > 255.0f) {
        return 255;
    }
    return (uint8_t)lrintf(v);
}

/* Приближенное преобразование CIE xy + яркость -> RGB */
static void xy_level_to_rgb(uint16_t x_u16, uint16_t y_u16, uint8_t level, uint8_t *r, uint8_t *g, uint8_t *b)
{
    float x = (float)x_u16 / 65535.0f;
    float y = (float)y_u16 / 65535.0f;

    if (y < 0.0001f) {
        y = 0.0001f;
    }

    float Y = (float)level / 254.0f;
    float X = (Y / y) * x;
    float Z = (Y / y) * (1.0f - x - y);

    float r_lin = X * 1.612f - Y * 0.203f - Z * 0.302f;
    float g_lin = -X * 0.509f + Y * 1.412f + Z * 0.066f;
    float b_lin = X * 0.026f - Y * 0.072f + Z * 0.962f;

    if (r_lin < 0.0f) r_lin = 0.0f;
    if (g_lin < 0.0f) g_lin = 0.0f;
    if (b_lin < 0.0f) b_lin = 0.0f;

    float max_lin = fmaxf(r_lin, fmaxf(g_lin, b_lin));
    if (max_lin > 1.0f) {
        r_lin /= max_lin;
        g_lin /= max_lin;
        b_lin /= max_lin;
    }

    /* gamma correction */
    float r_gamma = powf(r_lin, 1.0f / 2.2f);
    float g_gamma = powf(g_lin, 1.0f / 2.2f);
    float b_gamma = powf(b_lin, 1.0f / 2.2f);

    *r = clamp_u8(r_gamma * 255.0f);
    *g = clamp_u8(g_gamma * 255.0f);
    *b = clamp_u8(b_gamma * 255.0f);
}

static void light_apply_state(void)
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;

    if (s_light.on && s_light.level > 0) {
        xy_level_to_rgb(s_light.x, s_light.y, s_light.level, &r, &g, &b);
    }

    s_light.r = r;
    s_light.g = g;
    s_light.b = b;

    if (s_led_strip) {
        led_strip_set_pixel(s_led_strip, 0, s_light.r, s_light.g, s_light.b);
        led_strip_refresh(s_led_strip);
    }

    ESP_LOGI(TAG,
             "Applied RGB state: on=%d level=%u xy=(%u,%u) rgb=(%u,%u,%u)",
             s_light.on,
             s_light.level,
             s_light.x,
             s_light.y,
             s_light.r,
             s_light.g,
             s_light.b);
}

static __attribute__((unused)) void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ezb_bdb_start_top_level_commissioning(mode_mask);
}

static void pair_button_poll(void)
{
    int64_t now_us = esp_timer_get_time();
    bool raw_level = gpio_get_level(PAIR_BUTTON_GPIO);

    if (raw_level != s_button_raw_state) {
        s_button_raw_state = raw_level;
        s_button_last_change_us = now_us;
        return;
    }

    if ((now_us - s_button_last_change_us) < BUTTON_DEBOUNCE_US) {
        return;
    }

    if (raw_level == s_button_state) {
        return;
    }

    s_button_state = raw_level;
    if (!s_button_state) {
        if ((now_us - s_button_last_press_us) <= BUTTON_DOUBLE_CLICK_WINDOW_US) {
            s_button_click_count++;
        } else {
            s_button_click_count = 1;
        }
        s_button_last_press_us = now_us;

        if (s_button_click_count >= 2) {
            ESP_LOGI(TAG, "Pair button double-clicked, starting direct binding (finding & binding)");
            ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_FINDING_N_BINDING);
            s_button_click_count = 0;
        }
    }

    if (s_button_click_count == 1 && (now_us - s_button_last_press_us) > BUTTON_DOUBLE_CLICK_WINDOW_US) {
        ESP_LOGI(TAG, "Pair button single-clicked, starting simple binding (network steering)");
        ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_NETWORK_STEERING);
        s_button_click_count = 0;
    }
}

/* EZB API variants use different attribute callback types/layouts.
 * Keep full attribute handling for ESP_ZB API and use a safe no-op handler on EZB builds.
 */
static __attribute__((unused)) esp_err_t zb_attribute_handler(const void *message)
{
    (void)message;
    return ESP_OK;
}
#endif

static void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    if (!signal_struct || !signal_struct->p_app_signal) {
        ESP_LOGE(TAG, "Invalid Zigbee signal pointer");
        return;
    }

    uint32_t sig_type = *(uint32_t *)signal_struct->p_app_signal;
    esp_err_t status = signal_struct->esp_err_status;

    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Zigbee stack initialized, starting commissioning");
        esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb,
                               ESP_ZB_BDB_MODE_INITIALIZATION,
                               100);
        break;

    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (status == ESP_OK) {
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Factory-new device, starting network steering");
                ezb_bdb_start_top_level_commissioning(EZB_BDB_MODE_NETWORK_STEERING);
            } else {
                ESP_LOGI(TAG, "Device rebooted and joined network");
            }
        } else {
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack: %s", esp_err_to_name(status));
        }
        break;

    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (status == ESP_OK) {
            esp_zb_ieee_addr_t ieee_addr;
            esp_zb_get_long_address(ieee_addr);
            ESP_LOGI(TAG,
                     "Joined network successfully (IEEE: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x)",
                     ieee_addr[7], ieee_addr[6], ieee_addr[5], ieee_addr[4],
                     ieee_addr[3], ieee_addr[2], ieee_addr[1], ieee_addr[0]);
        } else {
            ESP_LOGW(TAG, "Network steering failed, retrying");
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb,
                                   ESP_ZB_BDB_MODE_NETWORK_STEERING,
                                   1000);
        }
        break;

    default:
        ESP_LOGI(TAG, "Zigbee signal: %lu status: %s", (unsigned long)sig_type, esp_err_to_name(status));
        break;
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Firmware version: %s", FIRMWARE_VERSION);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    ESP_ERROR_CHECK(rgb_led_init());
    ESP_ERROR_CHECK(pair_button_init());

    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    zb_nwk_cfg.install_code_policy = INSTALL_CODE_POLICY_ENABLE;
    esp_zb_init(&zb_nwk_cfg);

    esp_zb_color_dimmable_light_cfg_t light_cfg = ESP_ZB_DEFAULT_COLOR_DIMMABLE_LIGHT_CONFIG();
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();
    esp_zb_cluster_list_t *cluster_list = esp_zb_color_dimmable_light_clusters_create(&light_cfg);

    esp_zb_basic_cluster_add_attr(cluster_list,
                                  ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID,
                                  (void *)"ESPRESSIF");
    esp_zb_basic_cluster_add_attr(cluster_list,
                                  ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,
                                  (void *)"ESP32C6_SUPERMINI_RGB");

#if defined(EZB_AF_HA_PROFILE_ID)
    (void)light_cfg;
    (void)ep_list;
    (void)cluster_list;
    ESP_LOGW(TAG, "EZB SDK detected: legacy endpoint registration path is not implemented in this sample");
    return;
#else
    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = HA_ESP_LIGHT_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_COLOR_DIMMABLE_LIGHT_DEVICE_ID,
        .app_device_version = 0,
    };

    esp_zb_ep_list_add_ep(ep_list, cluster_list, endpoint_config);
    esp_zb_device_register(ep_list);

    ESP_LOGW(TAG, "Attribute callback disabled in compatibility build");
    light_apply_state();

    ESP_ERROR_CHECK(esp_zb_start(false));
    while (1) {
        esp_zb_main_loop_iteration();
        pair_button_poll();
    }
#endif
}

#include <string.h>

#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_zigbee_core.h"
#include "ha/esp_zigbee_ha_standard.h"

static const char *TAG = "zha_light";

#define HA_ESP_LIGHT_ENDPOINT 10
#define INSTALL_CODE_POLICY_ENABLE false

static bool s_light_on = false;

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    esp_zb_bdb_start_top_level_commissioning(mode_mask);
}

static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    if (!message || !message->info.status) {
        return ESP_ERR_INVALID_ARG;
    }

    if (message->info.dst_endpoint == HA_ESP_LIGHT_ENDPOINT &&
        message->info.cluster == ESP_ZB_ZCL_CLUSTER_ID_ON_OFF &&
        message->attribute.id == ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID &&
        message->attribute.data.type == ESP_ZB_ZCL_ATTR_TYPE_BOOL) {

        s_light_on = *(bool *)message->attribute.data.value;
        ESP_LOGI(TAG, "Light state updated from ZHA: %s", s_light_on ? "ON" : "OFF");
        // Здесь можно управлять GPIO реального реле/светодиода.
        return ESP_OK;
    }

    return ESP_OK;
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    uint32_t sig_type = *p_sg_p;
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
            ESP_LOGI(TAG, "Device started successfully, status: 0x%x", status);
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Factory-new device, starting network steering");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            } else {
                ESP_LOGI(TAG, "Device rebooted and joined network");
            }
        } else {
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(status));
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
            ESP_LOGW(TAG, "Network steering was not successful, retrying");
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
    ESP_ERROR_CHECK(nvs_flash_init());

    esp_zb_platform_config_t config = {
        .radio_config = ESP_ZB_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_ZB_DEFAULT_HOST_CONFIG(),
    };
    ESP_ERROR_CHECK(esp_zb_platform_config(&config));

    esp_zb_cfg_t zb_nwk_cfg = ESP_ZB_ZED_CONFIG();
    zb_nwk_cfg.install_code_policy = INSTALL_CODE_POLICY_ENABLE;

    esp_zb_init(&zb_nwk_cfg);

    esp_zb_on_off_light_cfg_t light_cfg = ESP_ZB_DEFAULT_ON_OFF_LIGHT_CONFIG();
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();
    esp_zb_cluster_list_t *cluster_list = esp_zb_on_off_light_clusters_create(&light_cfg);

    esp_zb_basic_cluster_add_attr(cluster_list,
                                  ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID,
                                  (void *)"ESPRESSIF");
    esp_zb_basic_cluster_add_attr(cluster_list,
                                  ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID,
                                  (void *)"ESP32C6_ZHA_LIGHT");

    esp_zb_endpoint_config_t endpoint_config = {
        .endpoint = HA_ESP_LIGHT_ENDPOINT,
        .app_profile_id = ESP_ZB_AF_HA_PROFILE_ID,
        .app_device_id = ESP_ZB_HA_ON_OFF_LIGHT_DEVICE_ID,
        .app_device_version = 0,
    };

    esp_zb_ep_list_add_ep(ep_list, cluster_list, endpoint_config);
    esp_zb_device_register(ep_list);

    esp_zb_zcl_register_set_attr_value_cb(zb_attribute_handler);

    ESP_ERROR_CHECK(esp_zb_start(false));
    while (1) {
        esp_zb_main_loop_iteration();
    }
}

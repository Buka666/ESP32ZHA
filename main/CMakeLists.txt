idf_component_register(SRCS "zigbee_light.c"
                    INCLUDE_DIRS "."
                    REQUIRES led_strip esp_driver_gpio)

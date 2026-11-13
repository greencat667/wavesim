idf_component_register(SRCS "app.c" "../../../device/idf_hal.c" INCLUDE_DIRS "../../../core" "../../../device" REQUIRES driver esp_http_client esp_wifi nvs_flash esp_netif esp_timer mbedtls)
target_compile_definitions(${COMPONENT_LIB} PRIVATE WAVE_URL="${CONFIG_WAVE_HTTP_URL}")

#pragma once

#include "led_strip.h"
#include "driver/gpio.h"
#include "color_codes.h"

#define LED_TYPE LED_STRIP_SK6812
#ifdef CONFIG_IDF_TARGET_ESP32S3
#define LED_GPIO GPIO_NUM_47
#elif CONFIG_IDF_TARGET_ESP32
#define LED_GPIO GPIO_NUM_2
#endif
#define LED_STRIP_LEN 10

led_strip_t *sk6812_setup(led_strip_t *strip);
esp_err_t sk6812_config(led_strip_t *strip);
esp_err_t sk6812_set_color(led_strip_t *strip, uint32_t color_code);
esp_err_t sk6812_set_brightness(led_strip_t *strip, uint8_t brightness);
esp_err_t sk6812_show(led_strip_t *strip);

#include "driver/gpio.h"
#include "button.hpp"
#include "ssd1306.hpp"

#include "wifi.hpp"

#define pin_SCL GPIO_NUM_22
#define pin_SDA GPIO_NUM_21

uint8_t current_line = 0;
OLED oled = OLED(pin_SCL, pin_SDA, SSD1306_128x64);
ESPWifi wifi;

static uint8_t btns[] = {
    GPIO_NUM_32,
    GPIO_NUM_33,
    GPIO_NUM_34,
    GPIO_NUM_36,
    GPIO_NUM_39
    };

Button *pButtons[5];
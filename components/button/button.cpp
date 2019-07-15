#include "button.hpp"
#include "esp_log.h"
#include <time.h>

Button::Button(uint8_t button_pin, void (*callback)(Button *))
{
    pin = button_pin;
    press_callback = callback;
    init();
}
void Button::init()
{
    gpio_set_direction((gpio_num_t)pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode((gpio_num_t)pin, GPIO_PULLUP_ONLY);
}
void Button::loop()
{
    old_status = status;
    status = gpio_get_level((gpio_num_t)pin);
    if(old_status == HIGH && status == LOW)
    {
        push_time = millis();
        long_press = false;  
    } else
        if(old_status == LOW && status == HIGH)
        {
            pressed_time = millis() - push_time;
            if(pressed_time >= debonce_time && !long_press)
                press_callback(this);
            long_press = false;
        } else
            if(status == LOW && ((pressed_time = millis() - push_time) >= longpress_time))
            {
                push_time = millis();
                long_press = true;
                press_callback(this);
            } else
                if(status == LOW && long_press && ((pressed_time = millis() - push_time) >= autorepeat_time))
                {
                    push_time = millis();
                    press_callback(this);
                }
}
uint32_t Button::millis()
{
    return (uint32_t) (clock() * 1000 / CLOCKS_PER_SEC);
}
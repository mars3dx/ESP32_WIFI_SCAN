#include "esp_system.h"
#include "driver/gpio.h"
#define HIGH 1
#define LOW 0

class Button
{
    protected:
        uint32_t push_time;
        uint32_t pressed_time;
        void (*press_callback)(Button *);
        bool long_press = false;
        uint32_t debonce_time = 50;
        uint32_t longpress_time = 800;
        uint32_t autorepeat_time = 200;
    public:
        Button(uint8_t button_pin, void (*callback)(Button *));
        uint8_t pin;
        uint8_t old_status = HIGH;
        uint8_t status = HIGH;
        void init();
        void loop();
        uint32_t millis();
        uint32_t getPressedTime(){return pressed_time;};       
};

#include "esp_log.h"
#include "esp_system.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "main.hpp"
#include <string>
#include <list>

using namespace std;

static const char TAG[] = "esp_sta";

void info_screen(char *message)
{
    oled.clear();
    oled.draw_string(63 - oled.measure_string(message) / 2, 32 - oled.get_font_height() / 2, message, WHITE, BLACK);
    oled.refresh(false);
}

void wirteln(string buf)
{
    if(current_line > 54)
    {
        oled.clear();
        current_line = 0;   
    }
    oled.fill_rectangle(0, current_line, oled.get_width() - 1, oled.get_font_height(), BLACK);
    oled.draw_string(0, current_line, buf, WHITE, BLACK);
    oled.refresh(false);
    current_line += oled.get_font_height() + 2;
}

void buttonTask(void *pParam)
{
    while(1)
    {
        for(int i=0;i<5;i++)  
            pButtons[i]->loop();
    }
}

int ap_list_line = 0;
int sel_line = 0;

void show_ap(int n = 0, int sel = 0)
{
    oled.clear();
    current_line = 0;
    for(int i=0; i < (wifi.info.size() > 7 ? 7 : wifi.info.size()); i++)
    {
        if(i == sel)
        {
            oled.fill_rectangle(0, current_line, 127, oled.get_font_height(), WHITE);
            oled.draw_string(0, current_line, wifi.info.at(i + n).ssid, BLACK, WHITE);
        }
        else
            oled.draw_string(0, current_line, wifi.info.at(i + n).ssid, WHITE, BLACK);
        current_line += oled.get_font_height() + 2;
    }
    oled.refresh(false);
}

void sc_callback(vector<wifi_info> *info)
{
    ap_list_line = 0;
    show_ap(0);
}

void buttons_callback(Button *obj)
{
    switch(obj->pin)
    {
        case GPIO_NUM_32:
            ap_list_line = 0;
            sel_line = 0;
            oled.select_font(1);
            info_screen("Scaning...");
            oled.select_font(0);
            wifi.wifi_scan(sc_callback);
        break;
        case GPIO_NUM_33:
            show_ap(ap_list_line, sel_line);
        break;
        case GPIO_NUM_34:
            if(sel_line > 0)
                sel_line--;
            else
            if(ap_list_line > 0)
                ap_list_line--;
            show_ap(ap_list_line, sel_line);
        break;
        case GPIO_NUM_36:
            if(sel_line < 6)
                sel_line++;
            else
                if(ap_list_line < wifi.info.size() - 7)
                    ap_list_line++;
            show_ap(ap_list_line, sel_line);
        break;
        case GPIO_NUM_39:
            char bstr[255];
            
            oled.select_font(1);
            oled.clear();

            sprintf(bstr, "SSID: %s", wifi.info.at(ap_list_line + sel_line).ssid.c_str());
            oled.draw_string(0,0, bstr, WHITE, BLACK);
            sprintf(bstr, "BSSID: %x:%x:%x:%x:%x:%x",
                wifi.info.at(ap_list_line + sel_line).bssid[0],
                wifi.info.at(ap_list_line + sel_line).bssid[1],
                wifi.info.at(ap_list_line + sel_line).bssid[2],
                wifi.info.at(ap_list_line + sel_line).bssid[3],
                wifi.info.at(ap_list_line + sel_line).bssid[4],
                wifi.info.at(ap_list_line + sel_line).bssid[5]);
            oled.draw_string(0,11, bstr, WHITE, BLACK);
            sprintf(bstr, "RSSI: %i", wifi.info.at(ap_list_line + sel_line).rssi);
            oled.draw_string(0,22, bstr, WHITE, BLACK);
            sprintf(bstr, "CH: %i", wifi.info.at(ap_list_line + sel_line).channel);
            oled.draw_string(0,33, bstr, WHITE, BLACK);

            switch(wifi.info.at(ap_list_line + sel_line).authmode)
            {
                case WIFI_AUTH_OPEN:
                    sprintf(bstr, "AUTH: OPEN");                    
                break;
                case WIFI_AUTH_WEP:
                    sprintf(bstr, "AUTH: WEP");                    
                break;
                case WIFI_AUTH_WPA_PSK:
                    sprintf(bstr, "AUTH: WPA-PSK");                    
                break;
                case WIFI_AUTH_WPA2_PSK:
                    sprintf(bstr, "AUTH: WPA2-PSK");                    
                break;
                case WIFI_AUTH_WPA_WPA2_PSK:
                    sprintf(bstr, "AUTH: WPA/WPA2-PSK");                    
                break;
                case WIFI_AUTH_WPA2_ENTERPRISE:
                    sprintf(bstr, "AUTH: WPA2-ENTERPRISE");                    
                break;
                case WIFI_AUTH_MAX:
                    sprintf(bstr, "AUTH: MAX");                    
                break;
            }
            oled.draw_string(0, 44, bstr, WHITE, BLACK);
            
            oled.refresh(false);
            oled.select_font(0);
        break;
    }
    //ESP_LOGI(TAG, "GPIO: %i, time: %ims", obj->pin, obj->getPressedTime());
}

uint8_t ESPWifi::retry;

#ifdef __cplusplus
extern "C" {
#endif
    void app_main()
    {
        nvs_flash_init();
        wifi.init();

        if (oled.init())
        {
            oled.clear();
            oled.select_font(1);
            oled.invert_display(true);
            string str = "ESP32 OLED";
            oled.draw_string(63 - oled.measure_string(str) / 2, 28, str, WHITE, BLACK);
            oled.refresh(true);

            vTaskDelay(1000 / portTICK_PERIOD_MS);
            
            oled.invert_display(false);
            info_screen("Ready.");
            oled.select_font(0);

            for(int i=0;i<5;i++)
                pButtons[i] = new Button(btns[i], buttons_callback);

            TaskHandle_t tHandle;
            xTaskCreatePinnedToCore(buttonTask, "BUTTONS", 3096, NULL, tskIDLE_PRIORITY, &tHandle, 1);
        }
    }
#ifdef __cplusplus
}
#endif
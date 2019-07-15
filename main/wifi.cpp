#include "wifi.hpp"
#include "esp_wifi.h"
#include "esp_err.h"
#include "tcpip_adapter.h"
#include <string.h>

static const char TAG[] = "ESPWifi";

ESPWifi *pWifi;

ESPWifi::ESPWifi()
{
    pWifi = this;
}

int ESPWifi::init()
{
    tcpip_adapter_init();
    esp_event_loop_create_default();
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event, NULL);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event, NULL);
    
    strcpy(country.cc, "CN");
    country.schan = 1;
    country.nchan = 13;
    country.policy = WIFI_COUNTRY_POLICY_AUTO;
    esp_wifi_set_country(&country);
    esp_wifi_init(&cfg);

    // memset(&wifi_config, 0, sizeof(wifi_config_t));
    // strcpy((char*)wifi_config.sta.ssid, "ssid");
    // strcpy((char*)wifi_config.sta.password, "password");

    esp_wifi_set_mode(WIFI_MODE_STA);
    //esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);
    retry = 0;
    esp_wifi_start();

    return ESP_OK;
}

void ESPWifi::wifi_scan(void(*cb)(vector<wifi_info> *info))
{
    scan_callback = cb;
    wifi_scan_config_t scan_config;
    memset(&scan_config, 0, sizeof(wifi_scan_config_t));
    scan_config.scan_time = (wifi_scan_time_t){ .passive = 300 };
    scan_config.scan_type = WIFI_SCAN_TYPE_PASSIVE;

    esp_wifi_scan_start(&scan_config, false);
    ESP_LOGI(TAG, "Start scans...");
}

void ESPWifi::ip_event(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
    ESP_LOGI(TAG, "got ip:%s",
                ip4addr_ntoa(&event->ip_info.ip));  
}

void ESPWifi::wifi_event(void* arg, esp_event_base_t event_base, 
                                int32_t event_id, void* event_data)
{
    if (event_id == WIFI_EVENT_STA_START)
    {
        ESP_LOGI(TAG, "Connecting...");
        esp_wifi_connect();
        //pWifi->wifi_scan();
    }

    if (event_id ==  WIFI_EVENT_STA_DISCONNECTED)
    {
        if (retry < 5) {
            retry++;
            ESP_LOGI(TAG, "Retry: %i", retry);
            esp_wifi_connect();
        }
        else 
        {
            ESP_LOGI(TAG, "Not conect to wifi...");
        }
    }

    if (event_id == WIFI_EVENT_SCAN_DONE)
    {
        ESP_LOGI(TAG, "Scan done...");
        uint16_t nums = 25;
        wifi_ap_record_t *ap_records = (wifi_ap_record_t *)malloc(sizeof(wifi_ap_record_t) * nums);
        esp_wifi_scan_get_ap_records(&nums, ap_records);
        ESP_LOGI(TAG, "Find: %i ap's", nums);

        pWifi->info.clear();
        for(int i=0; i < nums; i++)
        {
            wifi_info wi;
            wi.channel = ap_records[i].primary;
            wi.rssi = ap_records[i].rssi;
            wi.ssid.assign((char *)(ap_records[i].ssid));
            memcpy(wi.bssid, ap_records[i].bssid, 6);
            wi.authmode = ap_records[i].authmode;

            pWifi->info.push_back(wi);
            ESP_LOGI(TAG, "AP: %s RSSI: %i Ch: %i", ap_records[i].ssid, ap_records[i].rssi, ap_records[i].primary);
        }

        free(ap_records);

        pWifi->scan_callback(&pWifi->info);
    }
}
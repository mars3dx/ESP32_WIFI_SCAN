#include <stdint.h>
#include <vector>
#include <string>

#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"

using namespace std;

struct wifi_info
{
    string ssid;
    int rssi;
    int channel;
    uint8_t bssid[6];
    wifi_auth_mode_t authmode;
};
 
class ESPWifi
{
    protected:
        wifi_country_t country;
        wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
        wifi_config_t wifi_config;
    public:
        ESPWifi();
        int init();
        void wifi_scan(void(*cb)(vector<wifi_info> *info));
        void push_info(wifi_info wi);
        int get_info_size();
        vector<wifi_info> info;
        void (*scan_callback)(vector<wifi_info> *info);
        static uint8_t retry;
        static void ip_event(void* arg, esp_event_base_t event_base,
                                        int32_t event_id, void* event_data);
        static void wifi_event(void* arg, esp_event_base_t event_base, 
                                        int32_t event_id, void* event_data);
};

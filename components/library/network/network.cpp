
#include "network.h"
#include "esp_log.h"
#include "esp_system.h"

#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "string.h"
#include "freertos/event_groups.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "lwip/sockets.h"
#include <lwip/netdb.h>
#include "esp_timer.h"



const char *NETTAG = "NETWORK";


bool Network::init(home_network_config_t *cnf)
{
    NetConfig = cnf;
    xEventGroupClearBits(Event, BTT0 | BTT1);
    return true;
}

void Network::tim_callback(void* arg)
{   
    Network *aa = (Network *)arg;   
    ESP_LOGI(NETTAG,"AP Kapatiliyor");
    esp_wifi_stop();
    aa->tim_stop();
    ESP_LOGW(NETTAG,"AP KAPATILDI");
}

void Network::ap_event_handler(void* handler_args, esp_event_base_t base, int32_t id, void* event_data)
{
    Network *net = (Network *)handler_args;
    if (id == WIFI_EVENT_AP_STACONNECTED || id == WIFI_EVENT_AP_STADISCONNECTED || id== WIFI_EVENT_AP_PROBEREQRECVED || id == WIFI_TIMER)
    {
        //printf("zamanı yenile\n");
        net->tim_stop();
        net->tim_start();
    }
}


void Network::tim_stop(void){
    if (qtimer!=NULL)
      if (esp_timer_is_active(qtimer)) esp_timer_stop(qtimer);
}
void Network::tim_start(void){
    if (qtimer!=NULL)
      if (!esp_timer_is_active(qtimer))
         ESP_ERROR_CHECK(esp_timer_start_periodic(qtimer, 300 * 1000000));
}

bool Network::ap_init(void)
{
    esp_netif_t* wifiAP = esp_netif_create_default_wifi_ap(); 
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if(esp_wifi_init(&cfg)!=ESP_OK) return false;
    
    if(esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &ap_event_handler,
                                        (void *)this,
                                        NULL)!=ESP_OK) return false;
    
    wifi_ap_config_t aa = {};
    strcpy((char*)aa.ssid, "RoomBOXC2");
    strcpy((char*)aa.password, "roomboxc2");
    aa.ssid_len = strlen("RoomBOXC2");

    aa.channel = DEFAULT_AP_CHANNEL;
    aa.max_connection = AP_MAX_STA_CONN;
    aa.authmode = WIFI_AUTH_WPA_WPA2_PSK;
   
    wifi_config_t wifi_config = {
        .ap = aa,
    };

    esp_netif_dhcps_stop(wifiAP);
    esp_netif_ip_info_t info_t;
    memset(&info_t, 0, sizeof(esp_netif_ip_info_t));
        strcpy((char *)NetConfig->ip,"10.10.1.1");
        strcpy((char *)NetConfig->netmask,"255.255.255.0");
        strcpy((char *)NetConfig->gateway,"10.10.1.1");

        info_t.ip.addr = esp_ip4addr_aton((const char *)NetConfig->ip);
        info_t.netmask.addr = esp_ip4addr_aton((const char *) NetConfig->netmask);
        info_t.gw.addr = esp_ip4addr_aton((const char *) NetConfig->gateway);

    esp_netif_set_ip_info(wifiAP, &info_t);
    esp_netif_dhcps_start(wifiAP);

    if(esp_wifi_set_mode(WIFI_MODE_AP)!=ESP_OK) return false;
    if(esp_wifi_set_config(WIFI_IF_AP, &wifi_config)!=ESP_OK) return false;
    if(esp_wifi_start()!=ESP_OK) return false;

    NetConfig->home_ip = esp_ip4addr_aton((const char *)NetConfig->ip);
    NetConfig->home_netmask = esp_ip4addr_aton((const char *)NetConfig->netmask);
    NetConfig->home_gateway = esp_ip4addr_aton((const char *)NetConfig->gateway);
    NetConfig->home_broadcast = NetConfig->home_ip | (uint32_t)0xFF000000UL;

    xEventGroupSetBits(Event, WIFI_CONNECTED_BIT);
    esp_timer_create_args_t arg = {};

        arg.callback = &tim_callback;
        arg.name = "Ltim0";
        arg.arg = (void *) this;
        ESP_ERROR_CHECK(esp_timer_create(&arg, &qtimer)); 
        tim_start();

    return true;
}


bool Network::sta_init(void)
{
    esp_netif_t *_sta = esp_netif_create_default_wifi_sta();

    if (NetConfig->ipstat==STATIC_IP)
          {
            esp_netif_ip_info_t info_t;
            memset(&info_t, 0, sizeof(esp_netif_ip_info_t));
            esp_netif_dhcpc_stop(_sta);
            info_t.ip.addr = esp_ip4addr_aton((const char *)NetConfig->ip);
            info_t.netmask.addr = esp_ip4addr_aton((const char *)NetConfig->netmask);
            info_t.gw.addr = esp_ip4addr_aton((const char *)NetConfig->gateway);
            esp_netif_set_ip_info(_sta, &info_t);
          }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    if(esp_wifi_init(&cfg)!=ESP_OK) return false;
    wifi_sta_config_t aa = {};
    strcpy((char*)aa.ssid, (char*)NetConfig->wifi_ssid);
    strcpy((char*)aa.password,  (char*)NetConfig->wifi_pass);
    wifi_config_t wi_config = {
        .sta = aa,
    };

    if(esp_wifi_set_mode(WIFI_MODE_STA) !=ESP_OK) return false;
    if(esp_wifi_set_config(WIFI_IF_STA, &wi_config) !=ESP_OK) return false;
    if(esp_wifi_start() !=ESP_OK) return false;
    return true;
}

/*
bool Network::start(void)
{
    bool donus = false;
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      if(nvs_flash_erase()!=ESP_OK) {
        ESP_LOGE(NETTAG,"nvs_flash_erase not sucsess !..");
        return false;
      }
      ret = nvs_flash_init();
    }
    if(ret!=ESP_OK) {
        ESP_LOGE(NETTAG,"NETWORK storage flash section not accessed !..");
        return false;
      };
    if(esp_netif_init()!=ESP_OK) {ESP_LOGE(NETTAG,"esp_netif_init ERROR "); return false;}
    if(esp_event_loop_create_default()!=ESP_OK) {ESP_LOGE(NETTAG,"esp_event_loop_create_default ERROR "); return false;}
   
    //Access Point
    if (NetConfig.wifi_type==HOME_WIFI_AP)
    {
        ap_init();
        EventBits_t bits = xEventGroupWaitBits(Event,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
        if (bits&WIFI_CONNECTED_BIT) {
            donus = true;
        }
        ESP_LOGW(NETTAG,"Access Point");
    } 
    //Access Station
    if (NetConfig.wifi_type==HOME_WIFI_STA)
    {
        xEventGroupClearBits(Event, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
        if (!sta_init()) {set_full_error(); return false;}
        EventBits_t bits = xEventGroupWaitBits(Event,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
            if (bits & WIFI_CONNECTED_BIT) {
                ESP_LOGW(NETTAG,"Access Point Connected");
                donus = true;
          } 
    }
    //Auto
    if (NetConfig.wifi_type==HOME_WIFI_AUTO)
    {
        ESP_LOGI(NETTAG,"Access Point AUTO MODE");
        //first = false;
        xEventGroupClearBits(Event, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
        ESP_LOGI(NETTAG,"    Access Station connecting..");
        if (!sta_init()) {set_full_error(); return false;}
        EventBits_t bits = xEventGroupWaitBits(Event,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdTRUE,
            pdFALSE,
            portMAX_DELAY);
            if (bits & WIFI_CONNECTED_BIT) {
                donus = true;
          } else if (bits & WIFI_FAIL_BIT) {
                ESP_LOGI(NETTAG,"    Failed to connect to access point. Creating an access point.");
                xEventGroupClearBits(Event, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);
                if (!ap_init()) {set_full_error(); return false;}

                EventBits_t bits = xEventGroupWaitBits(Event,
                    WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                    pdTRUE,
                    pdFALSE,
                    portMAX_DELAY);
                    if (bits & WIFI_CONNECTED_BIT) {
                        donus = true;
                    }; 
          } else {   
                set_full_error(); 
                return false;
          } 
    }
   return donus;
}
*/

bool Network::net_init(void)
{
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
	  if(nvs_flash_erase()!=ESP_OK) {
		ESP_LOGE(NETTAG,"nvs_flash_erase not sucsess !..");
		return false;
	  }
	  ret = nvs_flash_init();
	}
	if(ret!=ESP_OK) {
		ESP_LOGE(NETTAG,"NETWORK storage flash section not accessed !..");
		return false;
	  };
	if(esp_netif_init()!=ESP_OK) {ESP_LOGE(NETTAG,"esp_netif_init ERROR "); return false;}

	return true;
}

esp_err_t Network::Station_Start(void)
{
	if (!net_init()) return ESP_FAIL;
	if (!sta_init()) return ESP_FAIL;
	EventBits_t bits = xEventGroupWaitBits(Event,
	            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
	            pdTRUE,
	            pdFALSE,
	            portMAX_DELAY);
    if (bits & WIFI_FAIL_BIT) return ESP_FAIL;
	return ESP_OK;
}

esp_err_t Network::Ap_Start(void)
{
	if (!net_init()) return ESP_FAIL;
	if (!ap_init()) return ESP_FAIL;
	EventBits_t bits = xEventGroupWaitBits(Event,
	            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
	            pdTRUE,
	            pdFALSE,
	            portMAX_DELAY);
    if (bits & WIFI_FAIL_BIT) return ESP_FAIL;
	return ESP_OK;
}

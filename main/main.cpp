#include "esp_log.h"
#include "assert.h"
#include "esp_system.h"
#include "esp_event.h"
#include "storage.h"
#include "uart.h"
#include "jsontool.h"
#include "ESP32Time.h"
#include "master_i2c.h"
#include "dev_8574.h"
#include "rs485.h"
#include "htime.h"
#include "classes.h"
#include "esp_ota_ops.h"
#include "cihazlar.h"
#include "IPTool.h"
#include "network.h"
#include "telnet.h"

#define LED0 0
#define BUTTON1 34
#define WATER 27

#define CPU2_ID 254
#define CPU1_ID 253

static const char *TAG = "OTEL_EKCPU2";
#define GLOBAL_FILE "/config/global.bin"
#define NETWORK_FILE "/config/network.bin"

ESP_EVENT_DEFINE_BASE(RS485_DATA_EVENTS);
ESP_EVENT_DEFINE_BASE(MESSAGE_EVENTS);

home_network_config_t NetworkConfig = {};
home_global_config_t GlobalConfig = {};
IPAddr Addr = IPAddr();
Storage disk = Storage();
UART_config_t uart_cfg={};
RS485_config_t rs485_cfg={};
ESP32Time rtc(0);
UART uart = UART();
RS485 rs485 = RS485();
Master_i2c bus = Master_i2c();
Dev_8574 pcf[4] = {};
Dev_8574 pcf0 =  Dev_8574();
Dev_8574 pcf1 =  Dev_8574();
Dev_8574 pcf2 =  Dev_8574();
Cihazlar cihazlar = Cihazlar();

Network wifi = Network();
bool netstatus = false;
SemaphoreHandle_t register_ready;
SemaphoreHandle_t status_ready;
SemaphoreHandle_t IACK_ready;
SemaphoreHandle_t REGOK_ready;
SemaphoreHandle_t STATUS_ready;


void day_change(struct tm *tminfo);
void min_change(struct tm *tminfo);


#include "tool/config.cpp"
#include "tool/tool.cpp"

void day_change(struct tm *tminfo)
{
   ESP_LOGW(TAG,"GUN DEGISTI"); 
}

void min_change(struct tm *tminfo)
{
   //ESP_LOGW(TAG,"SAAT DEGISTI"); 
   if (tminfo->tm_hour==8 && tminfo->tm_min==1)
     {
        //dayclean yayınla
         ESP_ERROR_CHECK(esp_event_post(FUNCTION_IN_EVENTS, DAYCLEAN_ON, NULL, 0, portMAX_DELAY));
         ESP_LOGI(TAG,"DAYCLEAN_ON Yayinlandi"); 
     }
}


extern "C" void app_main()
{
    esp_log_level_set("wifi", ESP_LOG_NONE); 
    esp_log_level_set("wifi_init", ESP_LOG_NONE); 
    esp_log_level_set("gpio", ESP_LOG_NONE); 
    if(esp_event_loop_create_default()!=ESP_OK) {ESP_LOGE(TAG,"esp_event_loop_create_default ERROR "); }

    ESP_LOGW(TAG,"INITIALIZING....."); 
    config();
    if (GlobalConfig.start_value==1)
    {
      disk.list("/config", "*.*");
      wifi.init(&NetworkConfig);
      wifi.cihaz = &cihazlar;
      esp_err_t rt = wifi.Ap_Start();
      if (rt==ESP_OK) netstatus = true;
    }

    REGOK_ready = xSemaphoreCreateBinary();
    assert(REGOK_ready);

    
    while(true) 
    {
        vTaskDelay(10/portTICK_PERIOD_MS);
        if (gpio_get_level((gpio_num_t)BUTTON1)==0) {
            if (GlobalConfig.start_value==1)
            {
                GlobalConfig.start_value = 0;
            } else {
                GlobalConfig.start_value = 1;
            }   
            disk.write_file(GLOBAL_FILE,&GlobalConfig,sizeof(GlobalConfig),0);
            ESP_LOGW(TAG,"Baslangic degeri %d olarak degistirildi. CPU Resetleniyor.",GlobalConfig.start_value) ;
            vTaskDelay(2000/portTICK_PERIOD_MS);
            esp_restart();
        }
    }    
}
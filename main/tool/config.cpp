
void CPU1_Ping_Reset(uint8_t counter)
{
    ESP_LOGE(TAG,"CPU1 Cevap vermiyor %d",counter);
}

void global_default_config(void)
{
     GlobalConfig.home_default = 1; 
     strcpy((char*)GlobalConfig.device_name, "Room_EKCPU2");
     strcpy((char*)GlobalConfig.mqtt_server,"");
     strcpy((char*)GlobalConfig.license, "");
     GlobalConfig.mqtt_keepalive = 0; 
     GlobalConfig.start_value = 0;
     GlobalConfig.device_id = 254;
     GlobalConfig.http_start = 1;
     //GlobalConfig.tcp_start = 1;
     GlobalConfig.reset_servisi = 0;
     disk.file_control(GLOBAL_FILE);
     disk.write_file(GLOBAL_FILE,&GlobalConfig,sizeof(GlobalConfig),0);
}

void network_default_config(void)
{
     NetworkConfig.home_default = 1; 
     NetworkConfig.wifi_type = HOME_WIFI_AP;
     NetworkConfig.ipstat = DYNAMIC_IP;
     strcpy((char*)NetworkConfig.wifi_ssid, "Room_Device");
     strcpy((char*)NetworkConfig.wifi_pass, "room");
     strcpy((char*)NetworkConfig.ip,"192.168.7.1");
     strcpy((char*)NetworkConfig.netmask,"255.255.255.0");
     strcpy((char*)NetworkConfig.gateway,"192.168.7.1");
     
     //strcpy((char*)NetworkConfig.update_server,"icemqtt.com.tr");
     

     //NetworkConfig.wifi_type = HOME_WIFI_STA;
     //strcpy((char *)NetworkConfig.wifi_ssid,(char *)"IMS_YAZILIM");
     //strcpy((char *)NetworkConfig.wifi_pass,(char *)"mer6514a4c");

     disk.file_control(NETWORK_FILE);
     disk.write_file(NETWORK_FILE,&NetworkConfig,sizeof(NetworkConfig),0);
}


void config(void)
{
    gpio_config_t io_conf = {};
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pin_bit_mask = (1ULL<<LED0);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; 
    gpio_config(&io_conf);
    
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<BUTTON1);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE; 
    gpio_config(&io_conf);

    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL<<WATER);
    io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE; 
    gpio_config(&io_conf);

    for (int i=0;i<10;i++) {
        gpio_set_level((gpio_num_t)LED0, 1);
        vTaskDelay(100/portTICK_PERIOD_MS);
        gpio_set_level((gpio_num_t)LED0, 0);
        vTaskDelay(100/portTICK_PERIOD_MS);
    }
    gpio_set_level((gpio_num_t)LED0, 0);

    ESP_LOGI(TAG,"NVS Flash Init");
    esp_err_t ret = nvs_flash_init();
        if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
            ESP_ERROR_CHECK( nvs_flash_erase() );
            ret = nvs_flash_init();
        }
     ESP_ERROR_CHECK( ret );
    
    ESP_LOGI(TAG,"FFS Init");
    ret = disk.init();
    ESP_ERROR_CHECK (ret);

    disk.read_file(NETWORK_FILE,&NetworkConfig,sizeof(NetworkConfig), 0);
    if (NetworkConfig.home_default==0 ) {
        //Network ayarları diskte kayıtlı değil. Kaydet.
         network_default_config();
         disk.file_control(NETWORK_FILE);
         disk.read_file(NETWORK_FILE,&NetworkConfig,sizeof(NetworkConfig),0);
         if (NetworkConfig.home_default==0 ) ESP_LOGW(TAG, "Network Initilalize File ERROR !...");
    }
    
    disk.read_file(GLOBAL_FILE,&GlobalConfig,sizeof(GlobalConfig), 0);
    if (GlobalConfig.home_default==0 ) {
        //Global ayarlar diskte kayıtlı değil. Kaydet.
         global_default_config();
         disk.read_file(GLOBAL_FILE,&GlobalConfig,sizeof(GlobalConfig),0);
         if (GlobalConfig.home_default==0 ) printf( "\n\nGlobal Initilalize File ERROR !...\n\n");
    }
    


   // rs485_output_test();

    ESP_LOGI(TAG,"I2C BUS Init");
    bus.init_bus(GPIO_NUM_21,GPIO_NUM_22, (i2c_port_t)1);
    pcf[0] = pcf0;
    pcf[1] = pcf1;
    pcf[2] = pcf2;
    ESP_ERROR_CHECK(pcf[0].init_device(&bus,0x20));
    ESP_LOGI(TAG,"0x20 PCF8574 ile iletişim kuruldu");
    ESP_ERROR_CHECK(pcf[1].init_device(&bus,0x21));
    ESP_LOGI(TAG,"0x21 PCF8574 ile iletişim kuruldu");
    ESP_ERROR_CHECK(pcf[2].init_device(&bus,0x22));
    ESP_LOGI(TAG,"0x22 PCF8574 ile iletişim kuruldu ");

    
    rs485_cfg.uart_num = 1;
    rs485_cfg.dev_num  = CPU2_ID;
    rs485_cfg.rx_pin   = 25;
    rs485_cfg.tx_pin   = 26;
    rs485_cfg.oe_pin   = 13;
    rs485_cfg.baud     = 57600;
    rs485.initialize(&rs485_cfg, (gpio_num_t)-1);

}
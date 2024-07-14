

#define BIN_PATTERN "%c %c %c %c   %c %c %c %c"
#define BYTE_TO_BIN(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

bool out_pin_convert(uint8_t pin, uint8_t *pcfno, uint8_t *pcfpin)
{
  bool ret=false;
  switch (pin)
   {
      case 1: {*pcfno=1;*pcfpin=0;ret=true;break;}
      case 2: {*pcfno=1;*pcfpin=1;ret=true;break;}
      case 3: {*pcfno=1;*pcfpin=2;ret=true;break;}
      case 4: {*pcfno=1;*pcfpin=3;ret=true;break;}
      case 5: {*pcfno=1;*pcfpin=7;ret=true;break;}
      case 6: {*pcfno=1;*pcfpin=6;ret=true;break;} //role cekmiyor

      case 7: {*pcfno=0;*pcfpin=1;ret=true;break;}
      case 8: {*pcfno=0;*pcfpin=2;ret=true;break;}
      case 9: {*pcfno=0;*pcfpin=3;ret=true;break;}
      case 10: {*pcfno=0;*pcfpin=4;ret=true;break;}
      case 11: {*pcfno=0;*pcfpin=5;ret=true;break;}
      case 12: {*pcfno=0;*pcfpin=6;ret=true;break;} //led yanmıyor

      case 13: {*pcfno=4;*pcfpin=0;ret=true;break;} 
      case 14: {*pcfno=4;*pcfpin=1;ret=true;break;} 
      case 15: {*pcfno=4;*pcfpin=2;ret=true;break;} 
   }
  return ret;
}

bool in_pin_convert(uint8_t pin, uint8_t *pcfno, uint8_t *pcfpin)
{
  bool ret=false;
  if (pin>=1 && pin<=8) {
                          *pcfno=2;
                          *pcfpin=pin-1;
                          ret=true;
                        }
  if (pin==9) {*pcfno=0; *pcfpin=7; ret=true; }    
  if (pin==10) {*pcfno=0; *pcfpin=0; ret=true; }                 
  if (pin==11) {*pcfno=1; *pcfpin=4; ret=true; }
  if (pin==12) {*pcfno=1; *pcfpin=5; ret=true; }  

  if (pin==13) {*pcfno=4; *pcfpin=3; ret=true; }  
  if (pin==14) {*pcfno=4; *pcfpin=4; ret=true; }  
  if (pin==15) {*pcfno=4; *pcfpin=5; ret=true; }  
  if (pin==16) {*pcfno=4; *pcfpin=6; ret=true; }  
  if (pin==17) {*pcfno=4; *pcfpin=7; ret=true; }  

  return ret;
}



void test02(uint16_t a_delay2, Dev_8574 *pcf)
{
   uint8_t aa = 0x00;
   while (true)
   {
      pcf[0].port_write(aa);
      pcf[1].port_write(aa);

      if(aa==0xFF) {
        vTaskDelay(a_delay2/portTICK_PERIOD_MS);
        pcf[1].pin_write(0,0);
        vTaskDelay(a_delay2/portTICK_PERIOD_MS);
        pcf[1].pin_write(0,1);
      }
  
      vTaskDelay(a_delay2/portTICK_PERIOD_MS);
      if (aa==0x00) aa=0xFF; else aa=0x00;
   }    
}


void inout_test(Dev_8574 *pcf)
{
  #define BUTTON1 34
  #define WATER 27

    uint16_t  a_delay=800;
    uint16_t a_delay1=500;
    ESP_LOGW("TOOL","\n\nIN/OUT TEST BASLADI\n\n");
    esp_log_level_set("ANAKUTU_CPU2", ESP_LOG_NONE);


    for (int q=0;q<2;q++)
    {
      if (q==0) {a_delay=100;a_delay1=50;}
      if (q==1) {a_delay=500;a_delay1=500;}
      if (q==2) {a_delay=800;a_delay1=500;}
      if (q==3) {a_delay=1500;a_delay1=500;}
      uint8_t say = 13;
      #ifdef PCF_4
         say=16;
      #endif   

      for (int j=1;j<say;j++)
        {  
          uint8_t pc=0, pn =0;
          out_pin_convert(j,&pc,&pn);
          pcf[pc].pin_write(pn,0);
          if ((j%2)==0)
            {
                ESP_LOGI("TOOL","%d --> %02d <-- PCF %d pin=%d level=0",q,j,pc,pn);
            } else {
                ESP_LOGW("TOOL","%d %02d PCF %d pin=%d level=0",q,j,pc,pn);
            }    
          vTaskDelay(a_delay/portTICK_PERIOD_MS);
          pcf[pc].pin_write(pn,1);
          //ESP_LOGI(TOOL_TAG,"%02d PCF %d pin=%d level=1",j, pc,pn);
          vTaskDelay(a_delay1/portTICK_PERIOD_MS);
        } 
    }

    ESP_LOGE("TOOL","GIRISLERI GND'ye çekerek test ediniz. Cikis icin RESET\n");
    uint8_t val0=0, val1 = 0, val2=0, val00 = 0xff, val11=0xff, val22 = 0xff;//, val33=0xff;
    bool rep = true;
    while (rep)
      {  
        
        pcf[0].port_read(&val0);
        pcf[1].port_read(&val1);
        pcf[2].port_read(&val2);
        #ifdef PCF_4
          pcf[4].port_read(&val3);
        #endif

        if (val0!=val00) {
                 val00 = val0;
                 if (val0!=0xFF) ESP_LOGI("TOOL","PCF0 = %02X Flag " BIN_PATTERN,val0,BYTE_TO_BIN(val0));
                 pcf[0].port_write(0xFF);
                         }
        if (val1!=val11) {
                 val11 = val1;
                 if (val1!=0xFF) ESP_LOGI("TOOL","PCF1 = %02X Flag " BIN_PATTERN,val1,BYTE_TO_BIN(val1));
                 pcf[1].port_write(0xFF);
                         }  
        if (val2!=val22) {
                 val22 = val2;
                 if (val2!=0xFF) ESP_LOGI("TOOL","PCF2 = %02X Flag " BIN_PATTERN,val2,BYTE_TO_BIN(val2));
                 pcf[2].port_write(0xFF);
                         }    
        #ifdef PCF_4
          if (val3!=val33) {
                 val33 = val3;
                 if (val3!=0xFF) ESP_LOGI("TOOL","PCF3 = %02X Flag " BIN_PATTERN,val3,BYTE_TO_BIN(val3));
                          }  
        #endif                                      
        if (gpio_get_level((gpio_num_t)WATER)==1) {ESP_LOGI("TOOL","WATER UP");vTaskDelay(1000/portTICK_PERIOD_MS);}
        if (gpio_get_level((gpio_num_t)BUTTON1)==0) {
                              ESP_LOGI("TOOL","BUTTON 1 UP");
                              vTaskDelay(1000/portTICK_PERIOD_MS);
                              }
                              //rs485_output_test();

        vTaskDelay(50/portTICK_PERIOD_MS);
      }         
}


#include "rs485.h"

void rs485_output_test(void)
{
bool rep = true;
uint16_t counter = 0;

RS485_config_t rs485_cfg={};
rs485_cfg.uart_num = 1;
rs485_cfg.dev_num  = 253;
rs485_cfg.rx_pin   = 25;
rs485_cfg.tx_pin   = 26;
rs485_cfg.oe_pin   = 13;
rs485_cfg.baud     = 115200;

uart_driver_delete((uart_port_t)rs485_cfg.uart_num);

        uart_config_t uart_config = {};
        uart_config.baud_rate = 115200;
        uart_config.data_bits = UART_DATA_8_BITS;
        uart_config.parity = UART_PARITY_DISABLE;
        uart_config.stop_bits = UART_STOP_BITS_1;
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        uart_config.rx_flow_ctrl_thresh = 122;

ESP_ERROR_CHECK(uart_driver_install((uart_port_t)rs485_cfg.uart_num, BUF_SIZE * 2, 0, 0, NULL, 0));
ESP_ERROR_CHECK(uart_param_config((uart_port_t)rs485_cfg.uart_num, &uart_config));
ESP_ERROR_CHECK(uart_set_pin((uart_port_t)rs485_cfg.uart_num, rs485_cfg.tx_pin, rs485_cfg.rx_pin, rs485_cfg.oe_pin, UART_PIN_NO_CHANGE));
ESP_ERROR_CHECK(uart_set_mode((uart_port_t)rs485_cfg.uart_num, UART_MODE_RS485_HALF_DUPLEX));
ESP_ERROR_CHECK(uart_set_rx_timeout((uart_port_t)rs485_cfg.uart_num, 3));
//uint8_t* data = (uint8_t*) malloc(BUF_SIZE);
//echo_send(rscfg->uart_num, "Start RS485 Output test.\r\n", 24);
char * bff = (char *)malloc(10);
uint8_t count = 0;
while (rep)
{ 
  sprintf(bff,"%02d",counter++);
  if (counter>98) counter=0; 
  //echo_send(rscfg->uart_num, "A", 1);
  uart_write_bytes((uart_port_t)rs485_cfg.uart_num, bff, strlen(bff));
  uart_wait_tx_done((uart_port_t)rs485_cfg.uart_num, 10);

    ESP_LOGI("TOOL","%s", bff);
    fflush(stdout);

    if(count++>10) {
      count=0;
      //printf("\n>> ");fflush(stdout);
    }

  vTaskDelay(5/portTICK_PERIOD_MS);

        
      }
}

void rs485_input_test(void)
{
//bool rep = true;
//uint16_t counter = 0;

RS485_config_t rs485_cfg={};
rs485_cfg.uart_num = 1;
rs485_cfg.dev_num  = 253;
rs485_cfg.rx_pin   = 25;
rs485_cfg.tx_pin   = 26;
rs485_cfg.oe_pin   = 13;
rs485_cfg.baud     = 115200;

uart_driver_delete((uart_port_t)rs485_cfg.uart_num);

        uart_config_t uart_config = {};
        uart_config.baud_rate = 115200;
        uart_config.data_bits = UART_DATA_8_BITS;
        uart_config.parity = UART_PARITY_DISABLE;
        uart_config.stop_bits = UART_STOP_BITS_1;
        uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;
        uart_config.rx_flow_ctrl_thresh = 122;

ESP_ERROR_CHECK(uart_driver_install((uart_port_t)rs485_cfg.uart_num, BUF_SIZE * 2, 0, 0, NULL, 0));
ESP_ERROR_CHECK(uart_param_config((uart_port_t)rs485_cfg.uart_num, &uart_config));
ESP_ERROR_CHECK(uart_set_pin((uart_port_t)rs485_cfg.uart_num, rs485_cfg.tx_pin, rs485_cfg.rx_pin, rs485_cfg.oe_pin, UART_PIN_NO_CHANGE));
ESP_ERROR_CHECK(uart_set_mode((uart_port_t)rs485_cfg.uart_num, UART_MODE_RS485_HALF_DUPLEX));
ESP_ERROR_CHECK(uart_set_rx_timeout((uart_port_t)rs485_cfg.uart_num, 3));

char * bff = (char *)calloc(1,10);
uint8_t count = 0;

while (true)
{ 
  int len = uart_read_bytes((uart_port_t)rs485_cfg.uart_num, bff, 2, (100 / portTICK_PERIOD_MS));
  if (len > 0) {
    printf("%s ", bff);
    fflush(stdout);

    if(count++>10) {
      count=0;
      printf("\n>> ");fflush(stdout);
    }
  } else ESP_ERROR_CHECK(uart_wait_tx_done((uart_port_t)rs485_cfg.uart_num, 10));

}

}

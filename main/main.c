#include <stdio.h>
#include <string.h>
#include <stdint.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "dhtHandler.h"
#include "adcSensorHandler.h"
#include "i2cdev.h"


// defines the GPIO pins 
#define DHT_GPIO 21

static const char TAG[] = "MAIN";

// variables 
adcSensorData_t adcData;
dhtSensorData_t dhtData;


// function prototypes 
static void adcEventCallback(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void dhtEventCallback(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
 

void app_main(void)
{
    esp_err_t err;
    esp_log_level_set("DHT_SENSOR", ESP_LOG_ERROR);
    esp_log_level_set("ADC_SENSOR", ESP_LOG_ERROR);


    ESP_LOGI(TAG, "System Start");

    // adcSensorConfig_t config = {
    //     .adcPinConfig.lightPin = 4, // pin 32
    //     .adcPinConfig.moisturePin = 5, // pin 33
    //     .adcTaskConfig.taskCore = -1,
    //     .adcTaskConfig.taskPrio = configMAX_PRIORITIES - 20,
    //     .adcTaskConfig.taskSize = configMINIMAL_STACK_SIZE * 4
    // };
    // err = adcSensorInitStack(&config, &adcEventCallback);
    // if(err != ESP_OK)
    //     ESP_LOGE(TAG, "ADC Stack Init ERROR!!!");

    

    // dhtSensorConfig_t dhtConfig = {
    //     .dhtPin = 21,
    //     .dhtType = DHT_TYPE_DHT11,
    //     .dhtTaskConfig.taskCore = -1,
    //     .dhtTaskConfig.taskPrio = configMAX_PRIORITIES - 21,
    //     .dhtTaskConfig.taskSize = configMINIMAL_STACK_SIZE * 3
    // };

    // err = dhtSensorInitStack(&dhtConfig, &dhtEventCallback);
    // if(err != ESP_OK)
    //     ESP_LOGE(TAG, "DHT Stack Init ERROR");


    i2c_dev_t dev = { 0 };
    dev.cfg.sda_io_num = 25;
    dev.cfg.scl_io_num = 26;
    dev.cfg.master.clk_speed = 100000;
    dev.port = 0;
    dev.addr = (0x7C >> 1);

    err = i2c_dev_create_mutex(&dev);
    if(err != ESP_OK)
        ESP_LOGE(TAG, "I2C Dev Create Error : Reason: %d || %s", err, esp_err_to_name(err));

    
    while(1)
    {
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }


}



static void adcEventCallback(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    
   // ESP_LOGI(TAG, "EventId: %" PRId32 "|| sensorDataLight: %d || SensorDataMoist: %d", event_id, adcData.sensorOne, adcData.sensorTwo);
    adcData = *(adcSensorData_t *)event_data;
    switch(event_id)
    {
        case ADC_LIGHT_SENSOR_EVENT:
            ESP_LOGI(TAG, "Light Sensor ADC: %d", adcData.sensorOne);
        break;

        case ADC_MOISTURE_SENSOR_EVENT:
            ESP_LOGI(TAG, "Moisture Sensor ADC: %d", adcData.sensorTwo);
        break;

    }
}

static void dhtEventCallback(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    // ESP_LOGI(TAG, "EventId: %" PRId32 "|| sensorDataLight: %d || SensorDataMoist: %d", event_id, adcData.sensorOne, adcData.sensorTwo);
    dhtData = *(dhtSensorData_t *)event_data;
    switch(event_id)
    {
        case DHT_TEMP_EVENT:
        ESP_LOGI(TAG, "Temperature: %.1fC",dhtData.temperature);
        break;

        case DHT_HUMIDITY_EVENT:
        ESP_LOGI(TAG, "Humidity: %.1f%%", dhtData.humidity);
        break;
    }
}
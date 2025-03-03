#include "adcSensorHandler.h"
#include <math.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


// adc related 
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "soc/soc_caps.h"

#include "esp_log.h"


static const char TAG[] = "ADC_SENSOR";

ESP_EVENT_DEFINE_BASE(ESP_ADC_SENSOR_EVENT);


TaskHandle_t adcTaskHandler;

#define EXAMPLE_ADC_ATTEN           ADC_ATTEN_DB_12


static adcSensorData_t rawSensorData;
static adcSensorPinConfig_t localPinConfig;
adc_oneshot_unit_handle_t adc1_handle;

#define GAMMA 0.7
#define RL10 33




void adcSensorTask(void *pvParameters);
bool checkDifference(int newValue, int prevValue, int maxDiff);

// global function 
esp_err_t adcSensorInitStack(adcSensorConfig_t *config, esp_event_handler_t adcSensorEventCallback)
{
    esp_err_t err;
    ESP_LOGI(TAG, "ADC Sensor Stack Initializing");

    err = esp_event_loop_create_default();
    if(err != ESP_OK)
        ESP_LOGE(TAG, "Failed To Setup eventLoop Reason: %d || %s", err, esp_err_to_name(err));

    err = esp_event_handler_register(ESP_ADC_SENSOR_EVENT, ESP_EVENT_ANY_ID, adcSensorEventCallback, NULL);
    if(err != ESP_OK)
        ESP_LOGE(TAG, "Failed to setup eventHandler Reason: %d || %s", err, esp_err_to_name(err));

    
    localPinConfig.lightPin = config->adcPinConfig.lightPin;
    localPinConfig.moisturePin = config->adcPinConfig.moisturePin;

    if(config->adcTaskConfig.taskCore == -1)
    {
        ESP_LOGI(TAG, "Starting ADC Task With No Core, StackSize: %d, Priority: %d", config->adcTaskConfig.taskSize, config->adcTaskConfig.taskPrio);
        xTaskCreate(adcSensorTask, "adctask", config->adcTaskConfig.taskSize, NULL, config->adcTaskConfig.taskPrio, &adcTaskHandler);
    }
    else 
    {
        ESP_LOGI(TAG, "Starting ADC Task With Core: %d, StackSize: %d, Priority: %d",config->adcTaskConfig.taskCore, config->adcTaskConfig.taskSize, config->adcTaskConfig.taskPrio);
        xTaskCreatePinnedToCore(adcSensorTask, "adctask", config->adcTaskConfig.taskSize, NULL, config->adcTaskConfig.taskPrio, &adcTaskHandler, config->adcTaskConfig.taskCore);
    }

    return err;
}

esp_err_t adcSensorDeinitStack(adcSensorConfig_t *config, esp_event_handler_t adcSensorEventCallback)
{
    esp_err_t err;
    ESP_LOGW(TAG, "ADC Sensor Stack De-initializing");

    err = esp_event_handler_unregister_with(adcSensorEventCallback, ESP_ADC_SENSOR_EVENT, ESP_EVENT_ANY_ID, adcSensorEventCallback);
    if(err != ESP_OK)
        ESP_LOGE(TAG, "Failed to unregister event Reason: %d || %s", err, esp_err_to_name(err));

    // delete the adc task 
    vTaskDelete(adcTaskHandler);
    
    // delete the adc Handler 
    err = adc_oneshot_del_unit(adc1_handle);
    if(err != ESP_OK)
        ESP_LOGE(TAG, "Failed to delete ADC Reason: %d || %s", err, esp_err_to_name(err));

    
    return err;
}


void adcSensorTask(void *pvParameters)
{
    esp_err_t err;

    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };

    err = adc_oneshot_new_unit(&init_config1, &adc1_handle);
    if(err != ESP_OK)
        ESP_LOGE(TAG, "ADC Oneshot Unit Err: Reason: %d || %s", err, esp_err_to_name(err));
    

    //-------------ADC1 Config---------------//
    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_DEFAULT,
        .atten = EXAMPLE_ADC_ATTEN,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, localPinConfig.lightPin, &config));
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, localPinConfig.moisturePin, &config));

    static int lightPrevVal, moisturePrevVal;
    static int lightData, moistureData;
    float voltage, resistance, lux;
    while(1)
    {
        // Read the Light Sensor Value 
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, localPinConfig.lightPin, &lightData));
        
        if(checkDifference(lightData, lightPrevVal, 10))
        {
            lightPrevVal = lightData;
            voltage = lightData / 4096.00 * 3.3;
            resistance = 2000 * voltage / (1 - voltage / 3.3);
            lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
            rawSensorData.sensorOne = lux;
            ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d || Lux: %.2f", ADC_UNIT_1 + 1, localPinConfig.lightPin, lightData, lux);
            //TODO: add event post function over here.... 
            ESP_ERROR_CHECK(esp_event_post(ESP_ADC_SENSOR_EVENT, ADC_LIGHT_SENSOR_EVENT, &rawSensorData, sizeof(adcSensorData_t), 100));

        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);

        // Read the Moisture sensor value 
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, localPinConfig.moisturePin, &moistureData));
        
        if(checkDifference(moistureData, moisturePrevVal, 5))
        {
            moisturePrevVal = moistureData;
            rawSensorData.sensorTwo = (100 - ((moistureData/4095.00) * 100));
            ESP_LOGI(TAG, "ADC%d Channel[%d] Raw Data: %d || Moisture: %d", ADC_UNIT_1 + 1, localPinConfig.moisturePin, moistureData, rawSensorData.sensorTwo);
            //TODO: add event post function over here.... 
            ESP_ERROR_CHECK(esp_event_post(ESP_ADC_SENSOR_EVENT, ADC_MOISTURE_SENSOR_EVENT, &rawSensorData, sizeof(adcSensorData_t), 100));
        }

        vTaskDelay(1000 / portTICK_PERIOD_MS);

    }

}


// helper functions
bool checkDifference(int newValue, int prevValue, int maxDiff) 
{
    return (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}

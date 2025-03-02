#include "dhtHandler.h"

#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"


static const char TAG[] = "DHT_SENSOR";

ESP_EVENT_DEFINE_BASE(ESP_DHT_SENSOR_EVENT);

TaskHandle_t dhtTaskHandler;

int dhtPinLocal;
dht_sensor_type_t dhtTypeLocal;
dhtSensorData_t rawDHTData;


void dhtSensorTask(void *pvParameters);
bool checkDiff(float newValue, float prevValue, float maxDiff);

// global function 
esp_err_t dhtSensorInitStack(dhtSensorConfig_t *config, esp_event_handler_t dhtSensorEventCallback)
{
    esp_err_t err;
    ESP_LOGI(TAG, "DHT Sensor Stack Initializing");

    err = esp_event_loop_create_default();
    if(err != ESP_OK)
        ESP_LOGE(TAG, "Failed To Setup eventLoop Reason: %d || %s", err, esp_err_to_name(err));

    err = esp_event_handler_register(ESP_DHT_SENSOR_EVENT, ESP_EVENT_ANY_ID, dhtSensorEventCallback, NULL);
    if(err != ESP_OK)
        ESP_LOGE(TAG, "Failed to setup eventHandler Reason: %d || %s", err, esp_err_to_name(err));

    
    dhtPinLocal = config->dhtPin;
    dhtTypeLocal = config->dhtType;

    if(config->dhtTaskConfig.taskCore == -1)
    {
        ESP_LOGI(TAG, "Starting DHT Task With No Core, StackSize: %d, Priority: %d", config->dhtTaskConfig.taskSize, config->dhtTaskConfig.taskPrio);
        xTaskCreate(dhtSensorTask, "dhttask", config->dhtTaskConfig.taskSize, NULL, config->dhtTaskConfig.taskPrio, &dhtTaskHandler);
    }
    else 
    {
        ESP_LOGI(TAG, "Starting DHT Task With Core: %d, StackSize: %d, Priority: %d",config->dhtTaskConfig.taskCore, config->dhtTaskConfig.taskSize, config->dhtTaskConfig.taskPrio);
        xTaskCreatePinnedToCore(dhtSensorTask, "dhttask", config->dhtTaskConfig.taskSize, NULL, config->dhtTaskConfig.taskPrio, &dhtTaskHandler, config->dhtTaskConfig.taskCore);
    }

    return err;
}

esp_err_t dhtSensorDeinitStack(dhtSensorConfig_t *config, esp_event_handler_t dhtSensorEventCallback)
{
    esp_err_t err;
    ESP_LOGW(TAG, "DHT Sensor Stack De-initializing");

    err = esp_event_handler_unregister_with(dhtSensorEventCallback, ESP_DHT_SENSOR_EVENT, ESP_EVENT_ANY_ID, dhtSensorEventCallback);
    if(err != ESP_OK)
        ESP_LOGE(TAG, "Failed to unregister event Reason: %d || %s", err, esp_err_to_name(err));

    // delete the dht task 
    vTaskDelete(dhtTaskHandler);

    return err;
}

void dhtSensorTask(void *pvParameters)
{
    float prevTemp = 00.00;
    float prevHumidity = 00.00;

    while(1)
    {
        if(dht_read_float_data(dhtTypeLocal, dhtPinLocal, &rawDHTData.humidity, &rawDHTData.temperature) == ESP_OK)
        {
            if(checkDiff(rawDHTData.temperature, prevTemp, 1.00))
            {
                prevTemp = rawDHTData.temperature;
                ESP_LOGI(TAG, "Temperature: %.1fC",rawDHTData.temperature);
                ESP_ERROR_CHECK(esp_event_post(ESP_DHT_SENSOR_EVENT, DHT_TEMP_EVENT, &rawDHTData, sizeof(dhtSensorData_t), 100));
            }

            if(checkDiff(rawDHTData.humidity, prevHumidity, 2.00))
            {
                prevHumidity = rawDHTData.humidity;
                ESP_LOGI(TAG, "Humidity: %.1f%%", rawDHTData.humidity);
                ESP_ERROR_CHECK(esp_event_post(ESP_DHT_SENSOR_EVENT, DHT_HUMIDITY_EVENT, &rawDHTData, sizeof(dhtSensorData_t), 100));
            }
        }
        else 
            ESP_LOGE(TAG, "Sensor Error!!");

        
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

bool checkDiff(float newValue, float prevValue, float maxDiff) 
{
    return !isnan(newValue) && (newValue < prevValue - maxDiff || newValue > prevValue + maxDiff);
}
  
#ifndef _ADC_SENSOR_HANDLER_H_
#define _ADC_SENSOR_HANDLER_H_

#include <stdio.h>
#include <stdint.h>
#include "esp_event.h"
#include "esp_err.h"


// Defines for the event loop 
ESP_EVENT_DECLARE_BASE(ESP_ADC_SENSOR_EVENT);


enum 
{
    ADC_LIGHT_SENSOR_EVENT,
    ADC_MOISTURE_SENSOR_EVENT
};

typedef struct 
{
    int taskCore;
    int taskPrio;
    int taskSize;
} adcSensorTaskConfig_t;

typedef struct 
{
    int lightPin;
    int moisturePin;
} adcSensorPinConfig_t;

typedef struct 
{
    adcSensorTaskConfig_t adcTaskConfig;
    adcSensorPinConfig_t adcPinConfig;
} adcSensorConfig_t;

typedef struct 
{
    int sensorOne;
    int sensorTwo;
} adcSensorData_t;


// #define EXAMPLE_ADC1_CHAN0          ADC_CHANNEL_4       // 
// #define EXAMPLE_ADC1_CHAN1          ADC_CHANNEL_5


esp_err_t adcSensorInitStack(adcSensorConfig_t *config, esp_event_handler_t adcSensorEventCallback);
esp_err_t adcSensorDeinitStack(adcSensorConfig_t *config, esp_event_handler_t adcSensorEventCallback);

#endif 

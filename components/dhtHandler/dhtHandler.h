#ifndef _DHT_HANDLER_H_
#define _DHT_HANDLER_H_

#include <stdio.h>
#include <stdint.h>
#include "dht.h"
#include "esp_event.h"
#include "esp_err.h"

// Defines for the event loop 
ESP_EVENT_DECLARE_BASE(ESP_DHT_SENSOR_EVENT);

enum 
{
    DHT_TEMP_EVENT,
    DHT_HUMIDITY_EVENT
};


typedef struct 
{
    int taskCore;
    int taskPrio;
    int taskSize;
} dhtSensorTaskConfig_t;


typedef struct 
{
    dhtSensorTaskConfig_t dhtTaskConfig;
    int dhtPin;
    dht_sensor_type_t dhtType;
} dhtSensorConfig_t;

typedef struct 
{
    float temperature;
    float humidity;
} dhtSensorData_t;

esp_err_t dhtSensorInitStack(dhtSensorConfig_t *config, esp_event_handler_t dhtSensorEventCallback);
esp_err_t dhtSensorDeinitStack(dhtSensorConfig_t *config, esp_event_handler_t dhtSensorEventCallback);


#endif
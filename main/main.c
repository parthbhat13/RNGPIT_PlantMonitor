#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "esp_log.h"

#include "dhtHandler.h"
#include "adcSensorHandler.h"
#include "hd44780.h"
#include "pcf8574.h"



// defines the GPIO pins 
#define DHT_GPIO 21

static const char TAG[] = "MAIN";

// variables 
adcSensorData_t adcData;
dhtSensorData_t dhtData;
static i2c_dev_t pcf8574;
char moistureBuffer[25];
char lightBuffer[25];
char temperatureBuffer[25];
char humidityBuffer[25];

// function prototypes 
static void adcEventCallback(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void dhtEventCallback(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data);

void ftoa(double n, char *res, int afterpoint);
int intToStr(int x, char str[], int d);
void rever(char *str, int len);

void app_main(void)
{
    esp_err_t err;
    esp_log_level_set("DHT_SENSOR", ESP_LOG_ERROR);
    esp_log_level_set("ADC_SENSOR", ESP_LOG_ERROR);


    ESP_LOGI(TAG, "System Start");

    ESP_ERROR_CHECK(i2cdev_init());

    hd44780_t lcd = {
        .write_cb = write_lcd_data, // use callback to send data to LCD by I2C GPIO expander
        .font = HD44780_FONT_5X8,
        .lines = 4,
        .pins = {
            .rs = 0,
            .e  = 2,
            .d4 = 4,
            .d5 = 5,
            .d6 = 6,
            .d7 = 7,
            .bl = 3
        }
    };

    memset(&pcf8574, 0, sizeof(i2c_dev_t));
    ESP_ERROR_CHECK(pcf8574_init_desc(&pcf8574, 0x3f, 0, 26, 25));
    ESP_ERROR_CHECK(hd44780_init(&lcd));
    hd44780_switch_backlight(&lcd, true);

    hd44780_gotoxy(&lcd, 0, 0);
    hd44780_puts(&lcd,"RNGPIT Plant Monitor");
    hd44780_gotoxy(&lcd, 5, 1);
    hd44780_puts(&lcd,"IoT Based!");
    hd44780_gotoxy(&lcd, 0, 2);
    hd44780_puts(&lcd, "RAiMECH Aero Pvt Ltd");
    hd44780_gotoxy(&lcd, 7, 3);
    hd44780_puts(&lcd, "V-0.0.1");

    adcSensorConfig_t config = {
        .adcPinConfig.lightPin = 4, // pin 32
        .adcPinConfig.moisturePin = 5, // pin 33
        .adcTaskConfig.taskCore = -1,
        .adcTaskConfig.taskPrio = configMAX_PRIORITIES - 20,
        .adcTaskConfig.taskSize = configMINIMAL_STACK_SIZE * 4
    };
    err = adcSensorInitStack(&config, &adcEventCallback);
    if(err != ESP_OK)
        ESP_LOGE(TAG, "ADC Stack Init ERROR!!!");

    

    dhtSensorConfig_t dhtConfig = {
        .dhtPin = 21,
        .dhtType = DHT_TYPE_DHT11,
        .dhtTaskConfig.taskCore = -1,
        .dhtTaskConfig.taskPrio = configMAX_PRIORITIES - 21,
        .dhtTaskConfig.taskSize = configMINIMAL_STACK_SIZE * 3
    };

    err = dhtSensorInitStack(&dhtConfig, &dhtEventCallback);
    if(err != ESP_OK)
        ESP_LOGE(TAG, "DHT Stack Init ERROR");

    vTaskDelay(3000 / portTICK_PERIOD_MS);

    // clear the buffers 
    memset(lightBuffer, '\0', sizeof(lightBuffer));
    memset(moistureBuffer, '\0', sizeof(moistureBuffer));
    memset(temperatureBuffer, '\0', sizeof(temperatureBuffer));
    memset(humidityBuffer, '\0', sizeof(humidityBuffer));
    hd44780_clear(&lcd);
    
    while(1)
    {

        // print Light Level 
        hd44780_gotoxy(&lcd,0,0);
        hd44780_puts(&lcd, "Light LUX:");
        hd44780_gotoxy(&lcd,10,0);
        ftoa(adcData.sensorOne, lightBuffer, 2);
        
        if(adcData.sensorOne < 99.99)
        {
            hd44780_puts(&lcd, lightBuffer); 
            hd44780_gotoxy(&lcd, 15, 0);
            hd44780_puts(&lcd, "     ");
        }
        else if(adcData.sensorOne < 999.99)
        {
            hd44780_puts(&lcd, lightBuffer); 
            hd44780_gotoxy(&lcd, 16, 0);
            hd44780_puts(&lcd, "    ");
        }
        else 
        {
            hd44780_puts(&lcd, lightBuffer); 
        }
        
        // Print soil moisture 
        hd44780_gotoxy(&lcd, 0, 1);
        hd44780_puts(&lcd, "Soil Moisture:");
        hd44780_gotoxy(&lcd, 14, 1);
        intToStr(adcData.sensorTwo, moistureBuffer, 2);
        
        if(adcData.sensorTwo < 9)
        {
            hd44780_puts(&lcd, moistureBuffer);
            hd44780_gotoxy(&lcd, 16, 1);
            hd44780_puts(&lcd, "%  ");
        }
        else if (adcData.sensorTwo < 99)
        {
            hd44780_puts(&lcd, moistureBuffer);
            hd44780_gotoxy(&lcd, 16, 1);
            hd44780_puts(&lcd, "%  ");
        }
        else 
        {
            hd44780_puts(&lcd, moistureBuffer);
            hd44780_gotoxy(&lcd, 17, 1);
            hd44780_puts(&lcd, "%");
        }

        // print temperature 
        hd44780_gotoxy(&lcd, 0,2);
        hd44780_puts(&lcd, "Temperature:");
        hd44780_gotoxy(&lcd, 12, 2);
        ftoa(dhtData.temperature, temperatureBuffer, 1);
        hd44780_puts(&lcd, temperatureBuffer);
        hd44780_gotoxy(&lcd, 16, 2);
        hd44780_putc(&lcd, 0xDF);
        hd44780_putc(&lcd, 'C');

        // print Humidity
        hd44780_gotoxy(&lcd, 0,3);
        hd44780_puts(&lcd, "Humidity:");
        hd44780_gotoxy(&lcd, 9, 3);
        ftoa(dhtData.humidity, humidityBuffer, 1);
        hd44780_puts(&lcd, humidityBuffer);
        hd44780_gotoxy(&lcd, 13, 3);
        hd44780_putc(&lcd, '%');
        

        vTaskDelay(200 / portTICK_PERIOD_MS);
        // hd44780_clear(&lcd);
    }

}



static void adcEventCallback(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    
   // ESP_LOGI(TAG, "EventId: %" PRId32 "|| sensorDataLight: %d || SensorDataMoist: %d", event_id, adcData.sensorOne, adcData.sensorTwo);
    adcData = *(adcSensorData_t *)event_data;
    switch(event_id)
    {
        case ADC_LIGHT_SENSOR_EVENT:
            ESP_LOGI(TAG, "Light Sensor ADC: %.2f", adcData.sensorOne);
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

static esp_err_t write_lcd_data(const hd44780_t *lcd, uint8_t data)
{
    return pcf8574_port_write(&pcf8574, data);
}

// Converts a floating point number to string.
void ftoa(double n, char *res, int afterpoint) 
{
    // Extract integer part
    int ipart = (int)n;
    // Extract floating part
    double fpart = n - (float)ipart;
    // convert integer part to string
    int i = intToStr(ipart, res, 0);
    // check for display option after point
    if (afterpoint != 0)  {
      res[i] = '.';  // add dot
      // Get the value of fraction part upto given no.
      // of points after dot. The third parameter is needed
      // to handle cases like 233.007
      fpart = fpart * pow(10, afterpoint);
      intToStr((int)fpart, res + i + 1, afterpoint);
    }
}

// Converts a given integer x to string str[].  d is the number
// of digits required in output. If d is more than the number
// of digits in x, then 0s are added at the beginning.
int intToStr(int x, char str[], int d) 
{
    int i = 0;
    while (x) 
    {
        str[i++] = (x % 10) + '0';
        x = x / 10;
    }
    // If number of digits required is more, then
    // add 0s at the beginning
    while (i < d)
        str[i++] = '0';

    rever(str, i);
    str[i] = '\0';
    return i;
}
  
  
  
void rever(char *str, int len) 
{
    int i = 0, j = len - 1, temp;
    while (i < j) 
    {
        temp = str[i];
        str[i] = str[j];
        str[j] = temp;
        i++; j--;
    }
}
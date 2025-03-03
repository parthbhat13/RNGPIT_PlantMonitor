#RNGPIT Plant Monitoring System 

### Project Brief / Features 

Super Basic project which is built with esp32 and some basic sensors like 
- LDR for measuring Light 
- Moisture Sensor For measuring soil Moisture 
- Temperature and humidity Sensor 
- 20x4 i2c Based Character Display

### More Details

- Used IDF V5.2.2
Below is the file structure which we are using 

```
├── components
│   ├── adcSensorHandler
│   │      └── adcHandler.h
│   │      └── adcHandler.c
│   │      └── CMakeLists.txt
│   ├── dhtHandler
│   │      └── driver
│   │      │     └── dht.h
│   │      │     └── dht.c
│   │      └── dhtHandler.h
│   │      └── dhtHandler.c
│   │      └── CMakeLists.txt
│   ├── esp_idf_lib_helpers
│   │      └── CMakeLists.txt
│   │      └── component.mk
│   │      └── esp_idf_lib_helpers.h
│   │      └── ets_sys.h
│   ├── lcdHandler
│   │      └── hd44780
│   │      │     └── hd44780.h
│   │      │     └── hd44780.c
│   │      └── i2cdev
│   │      │     └── i2cdev.h
│   │      │     └── i2cdev.c
│   │      └── pcf8574
│   │      │     └── pcf8574.h
│   │      │     └── pcf8574.c
│   │      └── CMakeLists.txt
│   │      └── Kconfig.projbuild
│   │      └── dhtHandler.h
├── main
│   ├── CMakeLists.txt
│   └── main.c
├── CMakeLists.txt
├── sdkconfig
├── .gitignore
└── README.md
```

### Project By: RAiMECH Aero Pvt. Ltd.

idf_component_register (SRCS "i2cdev/i2cdev.c" "hd44780/hd44780.c" "pcf8574/pcf8574.c"
                        INCLUDE_DIRS "i2cdev/." "hd44780/." "pcf8574/." .
                        REQUIRES driver esp_idf_lib_helpers
                        )
if(${IDF_TARGET} STREQUAL esp8266)
    set(req esp8266 freertos log esp_idf_lib_helpers)
else()
    set(req driver freertos log esp_idf_lib_helpers esp_event)
endif()

idf_component_register(
    SRCS driver/dht.c dhtHandler.c
    INCLUDE_DIRS driver/. .
    REQUIRES ${req}
)

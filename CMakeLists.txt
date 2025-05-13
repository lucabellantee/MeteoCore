cmake_minimum_required(VERSION 3.20.0)
set(ZEPHYR_BASE $ENV{ZEPHYR_BASE})
find_package(Zephyr REQUIRED HINTS ${ZEPHYR_BASE})
project(weather_prediction)

target_sources(app PRIVATE
  src/main.c
  src/bme280.c
  src/rain_model.c
  src/esp32_comm.c
)

target_include_directories(app PRIVATE src)
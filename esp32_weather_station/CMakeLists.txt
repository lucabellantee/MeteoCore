# Minimum CMake version required
cmake_minimum_required(VERSION 3.20.0)

# Set Zephyr directory
set(ZEPHYR_BASE C:/Users/lucab/zephyrproject/zephyr)

# Find Zephyr package
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(esp32_weather_station)

# Add source files
target_sources(app PRIVATE
  src/main.c
)

# Include directories
target_include_directories(app PRIVATE src)
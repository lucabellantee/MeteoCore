# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(weather_station)

# Crea directory per cJSON se non esiste
file(MAKE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib/cJSON)

# Scarica cJSON se non esiste
if(NOT EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/lib/cJSON/cJSON.c)
  file(DOWNLOAD 
       https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.c
       ${CMAKE_CURRENT_SOURCE_DIR}/lib/cJSON/cJSON.c)
  file(DOWNLOAD 
       https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.h
       ${CMAKE_CURRENT_SOURCE_DIR}/lib/cJSON/cJSON.h)
endif()

FILE(GLOB app_sources src/*.c)
target_sources(app PRIVATE ${app_sources})

# Includi la libreria cJSON
target_include_directories(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/cJSON)
target_sources(app PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/lib/cJSON/cJSON.c)
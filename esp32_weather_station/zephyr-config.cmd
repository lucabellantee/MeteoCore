@echo off
:: Configura Zephyr con la Zephyr SDK
set ZEPHYR_BASE=C:\Users\lucab\zephyrproject\zephyr
set ZEPHYR_TOOLCHAIN_VARIANT=zephyr
set ZEPHYR_SDK_INSTALL_DIR=C:\zephyr-sdk-0.17.0

:: Aggiungi la toolchain Zephyr al PATH
set PATH=C:\zephyr-sdk-0.17.0\xtensa-espressif_esp32_zephyr-elf\bin;%PATH%

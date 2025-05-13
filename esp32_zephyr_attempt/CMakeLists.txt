cmake_minimum_required(VERSION 3.20.0)

# Specify the Zephyr base path
set(ZEPHYR_BASE C:/zephyrproject/zephyr)

# Find the Zephyr package
find_package(Zephyr REQUIRED HINTS ${ZEPHYR_BASE})

# Define the project
project(esp32_zephyr_attempt)

# Specify the source files
target_sources(app PRIVATE src/main.c)

# Optionally, include the source directory
target_include_directories(app PRIVATE src)

# Zephyr automatically includes Zephyr headers, so this is typically not necessary
# If needed, explicitly include Zephyr directories (though it's generally handled automatically)
# target_include_directories(app PRIVATE ${ZEPHYR_BASE}/include)

# Optional: To link any specific Zephyr libraries (if necessary)
# target_link_libraries(app PRIVATE zephyr)




# cmake -B build           Configure
# cmake --build build      Build 

cmake_minimum_required(VERSION 3.10)
project(brightness)

# Set Release as default build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Use all parallel cores to build
include(ProcessorCount)
ProcessorCount(NPROC)
if(NPROC EQUAL 0)
    set(NPROC 1)
endif()
set(CMAKE_BUILD_PARALLEL_LEVEL ${NPROC})


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_library(WIRINGPI_LIBRARY wiringPi)
if(NOT WIRINGPI_LIBRARY)
    message(FATAL_ERROR "WiringPi library not found!")
endif()

add_executable(brightness
    brightness.cpp
    veml7700.cpp
)


target_link_libraries(brightness ${WIRINGPI_LIBRARY})
target_compile_options(brightness PRIVATE -Wall -Wextra)

# cmake --install build
install(TARGETS brightness
    DESTINATION /opt/brightness
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
)


add_custom_target(test-auto
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/brightness --auto
    COMMAND sleep 2s
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/brightness 255 1000
    DEPENDS brightness
)


add_custom_target(test-manual
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/brightness 0 16000
    COMMAND sleep 2s
    COMMAND ${CMAKE_CURRENT_BINARY_DIR}/brightness 255 16000
    DEPENDS brightness
)

# cmake --build build --target test
add_custom_target(test
    DEPENDS test-auto test-manual
)

# Add debug configuration (cmake --build build --config Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Add release configuration with O3 optimization (cmake --build build --config Release)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")


# Full installation
add_custom_target(install-all
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_CURRENT_BINARY_DIR} --config Release
    COMMAND cd ${CMAKE_CURRENT_SOURCE_DIR} && ./add-systemd-service.sh
    DEPENDS brightness
) 
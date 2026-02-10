#include <wiringPiI2C.h>
#include <iostream>
#include <unistd.h>  // For sleep function
#include "veml7700.h"
#include "config.h"
#include <thread>
#include <iomanip>  // Для std::fixed и std::setprecision

// VEML7700 I2C address
#define VEML7700_ADDR 0x10

// VEML7700 register addresses
#define REG_ALS_CONF 0x00
#define REG_ALS_DATA 0x04

// Configuration values
#define ALS_GAIN_1 0x00  // Gain x1
#define ALS_IT_100MS 0x00  // Integration time 100ms

float readAmbientLight(int fd) {
    // Read raw ALS data (16-bit)
    int raw_data = wiringPiI2CReadReg16(fd, REG_ALS_DATA);
    if (raw_data < 0) {
        throw std::runtime_error("Failed to read from ALS data register.");
    }

    // Convert raw data to lux
    // For gain x1 and integration time 100ms, lux = raw_data * 0.0576
    return raw_data * 0.0576;
}

void showAmbientLight(const Config& cfg) {
    int fd = wiringPiI2CSetup(VEML7700_ADDR);
    if (fd == -1) {
        throw std::runtime_error("Failed to initialize I2C communication.");
    }

    // Configure the sensor
    int config = (ALS_GAIN_1 << 11) | (ALS_IT_100MS << 6);
    if (wiringPiI2CWriteReg16(fd, REG_ALS_CONF, config) < 0) {
        throw std::runtime_error("Failed to configure light sensor.");
    }

    // Wait for the first measurement
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // std::cout << "DEBUG: " << cfg.measurement_interval_ms << " ms" << std::endl;

    while (true) {
        try {
            float lux = readAmbientLight(fd);
            std::cout << "Ambient Light: " << std::fixed << std::setprecision(2) << lux << " lux" << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(cfg.measurement_interval_ms));
        } catch (const std::exception& e) {
            throw;
        }
    }
}

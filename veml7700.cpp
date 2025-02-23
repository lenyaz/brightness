#include <wiringPiI2C.h>
#include <iostream>
#include <unistd.h>  // For sleep function

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

int main() {
    int fd;

    // Initialize I2C communication
    fd = wiringPiI2CSetup(VEML7700_ADDR);
    if (fd == -1) {
        std::cerr << "Failed to initialize I2C communication." << std::endl;
        return 1;
    }

    // Configure the sensor: gain x1, integration time 100ms
    int config = (ALS_GAIN_1 << 11) | (ALS_IT_100MS << 6);
    if (wiringPiI2CWriteReg16(fd, REG_ALS_CONF, config) < 0) {
        std::cerr << "Failed to write to configuration register." << std::endl;
        return 1;
    }

    // Wait for the first measurement to be ready
    sleep(1);

    while (true) {
        try {
            float lux = readAmbientLight(fd);
            std::cout << "Lux: " << lux << std::endl;
        } catch (const std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 1;
        }
        
        sleep(1);
    }

    return 0;
}

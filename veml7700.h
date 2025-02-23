#pragma once
#include <wiringPiI2C.h>
#include <stdexcept>

// VEML7700 I2C address
#define VEML7700_ADDR 0x10

// VEML7700 register addresses
#define REG_ALS_CONF 0x00
#define REG_ALS_DATA 0x04

// Configuration values
#define ALS_GAIN_1 0x00  // Gain x1
#define ALS_IT_100MS 0x00  // Integration time 100ms

float readAmbientLight(int fd); 
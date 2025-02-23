#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include "veml7700.h"

const std::string BRIGHTNESS_PATH = "/sys/class/backlight/10-0045/brightness";

// Constants for brightness values
const int MAX_BRIGHTNESS = 255;
const int MIN_BRIGHTNESS = 1;
const int OFF_BRIGHTNESS = 0;

// Function to read the current brightness
int get_current_brightness() {
	std::ifstream brightness_file(BRIGHTNESS_PATH);
	int brightness = 0;
	if (brightness_file.is_open()) {
		brightness_file >> brightness;
		brightness_file.close();
	}
	else {
		std::cerr << "Error: Unable to read brightness! :(" << std::endl;

		std::cerr << "Bailing out..." << std::endl;
		std::terminate();
	}
	return brightness;
}

// Function to set the brightness
void set_brightness(int value) {
	std::ofstream brightness_file(BRIGHTNESS_PATH);
	if (brightness_file.is_open()) {
		brightness_file << value;
		brightness_file.close();
	}
	else {
		std::cerr << "Error: Unable to set brightness! ;(" << std::endl;

		std::cerr << "Bailing out..." << std::endl;
		std::terminate();
	}
}

// Function to parse brightness argument
int parse_brightness_argument(const std::string& arg) {
	if (arg == "max") return MAX_BRIGHTNESS;
	if (arg == "min") return MIN_BRIGHTNESS;
	if (arg == "off") return OFF_BRIGHTNESS;
	
	try {
		int brightness = std::stoi(arg);
		if (brightness < 0 || brightness > MAX_BRIGHTNESS) {
			throw std::out_of_range("Brightness out of range");
		}
		return brightness;
	} catch (const std::exception& e) {
		std::cerr << "Error: Invalid brightness value. Use a number between 0-255 or 'max'/'min'/'off'" << std::endl;
		std::terminate();
	}
}

// Function to smoothly transition brightness
void transition_brightness(int current_brightness, int target_brightness, int transition_time_ms) {
	int steps = std::abs(current_brightness - target_brightness);
	if (steps == 0) return;

	double sleep_time_ms = static_cast<double>(transition_time_ms) / steps;
	double brightness_step = (target_brightness - current_brightness) / static_cast<double>(steps);

	for (int i = 0; i <= steps; ++i) {
		int new_brightness = std::round(current_brightness + i * brightness_step);
		set_brightness(new_brightness);
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time_ms)));
	}

	// Ensure final brightness is set correctly
	set_brightness(target_brightness);
}

// Function to convert lux to brightness (5 lux -> 1, 100 lux -> 255)
int luxToBrightness(float lux) {
	if (lux <= 5.0f) return MIN_BRIGHTNESS;
	if (lux >= 100.0f) return MAX_BRIGHTNESS;
	
	// Linear interpolation between 5 lux and 100 lux
	float brightness = ((lux - 5.0f) / (100.0f - 5.0f)) * (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;
	return static_cast<int>(std::round(brightness));
}

// Function to handle auto brightness mode
void auto_brightness_mode() {
	int fd = wiringPiI2CSetup(VEML7700_ADDR);
	if (fd == -1) {
		std::cerr << "Error: Failed to initialize I2C communication." << std::endl;
		std::terminate();
	}

	// Configure the sensor
	int config = (ALS_GAIN_1 << 11) | (ALS_IT_100MS << 6);
	if (wiringPiI2CWriteReg16(fd, REG_ALS_CONF, config) < 0) {
		std::cerr << "Error: Failed to configure light sensor." << std::endl;
		std::terminate();
	}

	// Wait for the first measurement
	std::this_thread::sleep_for(std::chrono::milliseconds(100));

	try {
		float lux = readAmbientLight(fd);
		int target_brightness = luxToBrightness(lux);
		int current_brightness = get_current_brightness();
		
		// Use a 1-second transition for auto-brightness
		transition_brightness(current_brightness, target_brightness, 1000);
	} catch (const std::exception& e) {
		std::cerr << "Error reading light sensor: " << e.what() << std::endl;
		std::terminate();
	}
}

//Usage: ./brightness <target_brightness> <transition_time_ms> (e.g. ./brightness 255 1000)
//       ./brightness --auto
int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <target_brightness> [transition_time_ms]" << std::endl;
		std::cerr << "   or: " << argv[0] << " --auto" << std::endl;
		return 1;
	}

	std::string arg1(argv[1]);
	if (arg1 == "--auto") {
		auto_brightness_mode();
		return 0;
	}

	if (argc > 3) {
		std::cerr << "Usage: " << argv[0] << " <target_brightness> [transition_time_ms]" << std::endl;
		return 1;
	}

	int transition_time_ms = (argc == 3) ? std::stoi(argv[2]) : 0;
	int target_brightness = parse_brightness_argument(argv[1]);
	int current_brightness = get_current_brightness();

	transition_brightness(current_brightness, target_brightness, transition_time_ms);

	return 0;
}

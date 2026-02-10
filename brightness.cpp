#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include "veml7700.h"
#include <iomanip>

const std::string BRIGHTNESS_PATH = "/sys/class/backlight/10-0045/brightness";

// Constants for brightness values
const int MAX_BRIGHTNESS = 255;
const int MIN_BRIGHTNESS = 1;
const int OFF_BRIGHTNESS = 0;

// Constants for ambient light thresholds (in lux)
const float MIN_LUX_THRESHOLD = 15.0f;
const float MAX_LUX_THRESHOLD = 250.0f;


// Квадратичная функция (более мягкая)
float easeInOutQuad(float t) {
	return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
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
	int brightness_diff = target_brightness - current_brightness;

	for (int i = 0; i <= steps; ++i) {
		// Calculate progress (0.0 to 1.0)
		float progress = static_cast<float>(i) / steps;
		
		// Apply easing function
		float eased_progress = easeInOutQuad(progress);
		
		// Calculate new brightness using eased progress
		int new_brightness = std::round(current_brightness + brightness_diff * eased_progress);
		
		set_brightness(new_brightness);
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time_ms)));
	}

	// Ensure final brightness is set correctly
	set_brightness(target_brightness);
}

// Function to convert lux to brightness
int luxToBrightness(float lux) {
	if (lux <= MIN_LUX_THRESHOLD) return MIN_BRIGHTNESS;
	if (lux >= MAX_LUX_THRESHOLD) return MAX_BRIGHTNESS;
	
	// Linear interpolation between min and max thresholds
	float brightness = ((lux - MIN_LUX_THRESHOLD) / (MAX_LUX_THRESHOLD - MIN_LUX_THRESHOLD)) 
		* (MAX_BRIGHTNESS - MIN_BRIGHTNESS) + MIN_BRIGHTNESS;
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

	std::cout << "Auto-brightness daemon started. Press Ctrl+C to stop." << std::endl;

	while (true) {
		try {
			float lux = readAmbientLight(fd);
			int target_brightness = luxToBrightness(lux);
			int current_brightness = get_current_brightness();
			
			std::cout << "Ambient Light: " << std::fixed << std::setprecision(2) 
					  << lux << " lux, Setting brightness to: " << target_brightness << std::endl;
			
			// Use a shorter transition for continuous mode
			transition_brightness(current_brightness, target_brightness, AUTO_TRANSITION_TIME_MS);
			
			// Wait before next measurement
			std::this_thread::sleep_for(std::chrono::milliseconds(MEASUREMENT_INTERVAL_MS));
			
		} catch (const std::exception& e) {
			std::cerr << "Error reading light sensor: " << e.what() << std::endl;
			std::terminate();
		}
	}
}

//Usage: ./brightness <target_brightness> <transition_time_ms> (e.g. ./brightness 255 1000)
//       ./brightness --auto
int main(int argc, char* argv[]) {
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <target_brightness> [transition_time_ms]" << std::endl;
		std::cerr << "   or: " << argv[0] << " --auto" << std::endl;
		std::cerr << "   or: " << argv[0] << " --showlux" << std::endl;
		return 1;
	}

	std::string arg1(argv[1]);
	if (arg1 == "--showlux") {
		try {
			showAmbientLight();
		} catch (const std::exception& e) {
			std::cerr << "Error: " << e.what() << std::endl;
			return 1;
		}
		return 0;
	}

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

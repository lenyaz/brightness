#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include "veml7700.h"
#include "config.h"
#include "brightness_backend.h"
#include <iomanip>
#include <vector>
#include <stdexcept>


// Квадратичная функция (более мягкая)
float easeInOutQuad(float t) {
	return t < 0.5f ? 2.0f * t * t : -1.0f + (4.0f - 2.0f * t) * t;
}


// Function to parse brightness argument
int parse_brightness_argument(const std::string& arg, const Config& cfg) {
	if (arg == "max") return cfg.max_brightness;
	if (arg == "min") return cfg.min_brightness;
	if (arg == "off") return cfg.off_brightness;
	
	try {
		int brightness = std::stoi(arg);
		if (brightness < 0 || brightness > cfg.max_brightness) {
			throw std::out_of_range("Brightness out of range. Valid range: " + std::to_string(cfg.min_brightness) + " to " + std::to_string(cfg.max_brightness));
		}
		return brightness;
	} catch (const std::exception&) {
		throw std::invalid_argument("Invalid brightness value. Use a number between " + std::to_string(cfg.off_brightness) + " (off) to " + std::to_string(cfg.max_brightness) + " or 'max'/'min'/'off'");
	}
}

// Function to smoothly transition brightness
void transition_brightness(BrightnessBackend& backend, int current_brightness, int target_brightness, int transition_time_ms) {
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
		
		backend.set(new_brightness);
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time_ms)));
	}

	// Ensure final brightness is set correctly
	backend.set(target_brightness);
}

// Function to convert lux to brightness
int luxToBrightness(float lux, const Config& cfg) {
	if (lux <= cfg.min_lux_threshold) return cfg.min_brightness;
	if (lux >= cfg.max_lux_threshold) return cfg.max_brightness;
	
	// Linear interpolation between min and max thresholds
	float brightness = ((lux - cfg.min_lux_threshold) / (cfg.max_lux_threshold - cfg.min_lux_threshold))
		* (cfg.max_brightness - cfg.min_brightness) + cfg.min_brightness;
	return static_cast<int>(std::round(brightness));
}

// Function to handle auto brightness mode
void auto_brightness_mode(const Config& cfg, BrightnessBackend& backend) {
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

	std::cout << "Auto-brightness daemon started. Press Ctrl+C to stop." << std::endl;

	while (true) {
		try {
			float lux = readAmbientLight(fd);
			int target_brightness = luxToBrightness(lux, cfg);
			int current_brightness = backend.get_current();
			
			std::cout << "Ambient Light: " << std::fixed << std::setprecision(2)
					  << lux << " lux, Setting brightness to: " << target_brightness << std::endl;
			
			// Use a shorter transition for continuous mode
			transition_brightness(backend, current_brightness, target_brightness, cfg.auto_transition_time_ms);
			
			// Wait before next measurement
			std::this_thread::sleep_for(std::chrono::milliseconds(cfg.measurement_interval_ms));
			
		} catch (const std::exception& e) {
			std::cerr << "Error in auto mode: " << e.what() << std::endl;
			std::this_thread::sleep_for(std::chrono::milliseconds(cfg.measurement_interval_ms));
		}
	}
}

static void print_usage(const char* argv0) {
	std::cerr << "Usage: " << argv0 << " [--config <path>] <target_brightness> [transition_time_ms]" << std::endl;
	std::cerr << "   or: " << argv0 << " [--config <path>] --auto" << std::endl;
	std::cerr << "   or: " << argv0 << " [--config <path>] --showlux" << std::endl;
}

//Usage: ./brightness <target_brightness> <transition_time_ms> (e.g. ./brightness 255 1000)
//       ./brightness --auto
int main(int argc, char* argv[]) {
	try {
		if (argc < 2) {
			print_usage(argv[0]);
			return 1;
		}

		std::string config_path = "/etc/brightness/config.ini";
		std::vector<std::string> args;
		args.reserve(static_cast<size_t>(argc));

		for (int i = 1; i < argc; ++i) {
			std::string arg(argv[i]);
			if (arg == "--config") {
				if (i + 1 >= argc) {
					throw std::invalid_argument("--config requires a path");
				}
				config_path = argv[i + 1];
				++i;
				continue;
			}
			args.push_back(arg);
		}

		Config cfg;
		auto cfg_err = loadConfig(config_path, &cfg);
		if (cfg_err.has_value()) {
			throw std::runtime_error("Config error: " + cfg_err.value());
		}

		auto backend = createBrightnessBackend(cfg);

		if (args.empty()) {
			print_usage(argv[0]);
			return 1;
		}

		std::string arg1(args[0]);
		if (arg1 == "--showlux") {
			showAmbientLight(cfg);
			return 0;
		}

		if (arg1 == "--auto") {
			auto_brightness_mode(cfg, *backend);
			return 0;
		}

		if (args.size() > 2) {
			print_usage(argv[0]);
			return 1;
		}

		int transition_time_ms = (args.size() == 2) ? std::stoi(args[1]) : 0;
		int target_brightness = parse_brightness_argument(args[0], cfg);
		int current_brightness = backend->get_current();

		transition_brightness(*backend, current_brightness, target_brightness, transition_time_ms);

		return 0;
	} catch (const std::exception& e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return 1;
	}
}

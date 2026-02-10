#pragma once
#include <string>
#include "config.h"
#include <optional>

struct Config {
    std::string brightness_path = "/sys/class/backlight/10-0045/brightness";
    std::string backend = "sysfs"; //sysfs or gpio
    int gpio_pin = -1;
    int pwm_range = 100;

    int max_brightness = 255;
    int min_brightness = 1;
    int off_brightness = 0;

    float min_lux_threshold = 15.0f;
    float max_lux_threshold = 250.0f;

    int measurement_interval_ms = 200;
    int auto_transition_time_ms = 1000;

    float boost_coef = 0.077;
};

// Loads config from INI-style file. Missing file -> std::nullopt (use defaults).
// Returns error string on parse/validation failure.
std::optional<std::string> loadConfig(const std::string& path, Config* out);

// Validates config values; returns error string if invalid.
std::optional<std::string> validateConfig(const Config& cfg);

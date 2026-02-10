#include "config.h"
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <cctype>

static std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) {
        ++start;
    }
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) {
        --end;
    }
    return s.substr(start, end - start);
}

static bool parseInt(const std::string& v, int* out) {
    try {
        size_t idx = 0;
        int val = std::stoi(v, &idx, 10);
        if (idx != v.size()) return false;
        *out = val;
        return true;
    } catch (...) {
        return false;
    }
}

static bool parseFloat(const std::string& v, float* out) {
    try {
        size_t idx = 0;
        float val = std::stof(v, &idx);
        if (idx != v.size()) return false;
        *out = val;
        return true;
    } catch (...) {
        return false;
    }
}

static std::optional<std::string> applyKeyValue(const std::string& key, const std::string& value, Config* cfg) {
    if (key == "brightness_path") {
        cfg->brightness_path = value;
        return std::nullopt;
    }
    if (key == "backend") {
        cfg->backend = value;
        return std::nullopt;
    }
    if (key == "gpio_pin") {
        int v = 0;
        if (!parseInt(value, &v)) return std::string("Invalid integer for gpio_pin");
        cfg->gpio_pin = v;
        return std::nullopt;
    }
    if (key == "pwm_range") {
        int v = 0;
        if (!parseInt(value, &v)) return std::string("Invalid integer for pwm_range");
        cfg->pwm_range = v;
        return std::nullopt;
    }
    if (key == "max_brightness") {
        int v = 0;
        if (!parseInt(value, &v)) return std::string("Invalid integer for max_brightness");
        cfg->max_brightness = v;
        return std::nullopt;
    }
    if (key == "min_brightness") {
        int v = 0;
        if (!parseInt(value, &v)) return std::string("Invalid integer for min_brightness");
        cfg->min_brightness = v;
        return std::nullopt;
    }
    if (key == "off_brightness") {
        int v = 0;
        if (!parseInt(value, &v)) return std::string("Invalid integer for off_brightness");
        cfg->off_brightness = v;
        return std::nullopt;
    }
    if (key == "min_lux_threshold") {
        float v = 0.0f;
        if (!parseFloat(value, &v)) return std::string("Invalid float for min_lux_threshold");
        cfg->min_lux_threshold = v;
        return std::nullopt;
    }
    if (key == "max_lux_threshold") {
        float v = 0.0f;
        if (!parseFloat(value, &v)) return std::string("Invalid float for max_lux_threshold");
        cfg->max_lux_threshold = v;
        return std::nullopt;
    }
    if (key == "measurement_interval_ms") {
        int v = 0;
        if (!parseInt(value, &v)) return std::string("Invalid integer for measurement_interval_ms");
        cfg->measurement_interval_ms = v;
        return std::nullopt;
    }
    if (key == "auto_transition_time_ms") {
        int v = 0;
        if (!parseInt(value, &v)) return std::string("Invalid integer for auto_transition_time_ms");
        cfg->auto_transition_time_ms = v;
        return std::nullopt;
    }
    return std::string("Unknown config key: ") + key;
}

std::optional<std::string> loadConfig(const std::string& path, Config* out) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt; // Missing file: use defaults
    }

    std::string line;
    int line_no = 0;
    while (std::getline(file, line)) {
        ++line_no;
        std::string trimmed = trim(line);
        if (trimmed.empty()) continue;
        if (trimmed[0] == '#' || trimmed[0] == ';') continue;
        if (trimmed.front() == '[' && trimmed.back() == ']') continue; // ignore section headers

        size_t eq = trimmed.find('=');
        if (eq == std::string::npos) {
            return std::string("Invalid line (missing '=') at ") + std::to_string(line_no);
        }
        std::string key = trim(trimmed.substr(0, eq));
        std::string value = trim(trimmed.substr(eq + 1));
        if (key.empty()) {
            return std::string("Invalid key at line ") + std::to_string(line_no);
        }

        auto err = applyKeyValue(key, value, out);
        if (err.has_value()) {
            return std::string("Config error at line ") + std::to_string(line_no) + ": " + err.value();
        }
    }

    return validateConfig(*out);
}

std::optional<std::string> validateConfig(const Config& cfg) {
    if (cfg.backend != "sysfs" && cfg.backend != "gpio") {
        return std::string("backend must be 'sysfs' or 'gpio'");
    }
    if (cfg.backend == "sysfs" && cfg.brightness_path.empty()) {
        return std::string("brightness_path must not be empty for sysfs backend");
    }
    if (cfg.backend == "gpio") {
        if (cfg.gpio_pin < 0) return std::string("gpio_pin must be >= 0 for gpio backend");
        if (cfg.pwm_range <= 0) return std::string("pwm_range must be > 0 for gpio backend");
    }
    if (cfg.max_brightness <= 0) return std::string("max_brightness must be > 0");
    if (cfg.min_brightness < 0) return std::string("min_brightness must be >= 0");
    if (cfg.off_brightness < 0) return std::string("off_brightness must be >= 0");
    if (cfg.min_brightness > cfg.max_brightness) return std::string("min_brightness must be <= max_brightness");
    if (cfg.off_brightness > cfg.max_brightness) return std::string("off_brightness must be <= max_brightness");
    if (cfg.min_lux_threshold < 0.0f || cfg.max_lux_threshold < 0.0f) {
        return std::string("lux thresholds must be >= 0");
    }
    if (cfg.min_lux_threshold > cfg.max_lux_threshold) {
        return std::string("min_lux_threshold must be <= max_lux_threshold");
    }
    if (cfg.measurement_interval_ms <= 0) return std::string("measurement_interval_ms must be > 0");
    if (cfg.auto_transition_time_ms < 0) return std::string("auto_transition_time_ms must be >= 0");
    return std::nullopt;
}

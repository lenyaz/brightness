#include "brightness_backend.h"
#include "config.h"
#include <fstream>
#include <stdexcept>
#include <algorithm>
#include <wiringPi.h>
#include <softPwm.h>

namespace {

class SysfsBackend : public BrightnessBackend {
public:
    explicit SysfsBackend(std::string path) : path_(std::move(path)) {}

    int get_current() const override {
        std::ifstream file(path_);
        int value = 0;
        if (!file.is_open()) {
            throw std::runtime_error("Unable to read brightness at " + path_);
        }
        file >> value;
        return value;
    }

    void set(int value) override {
        std::ofstream file(path_);
        if (!file.is_open()) {
            throw std::runtime_error("Unable to set brightness at " + path_);
        }
        file << value;
    }

private:
    std::string path_;
};

class GpioPwmBackend : public BrightnessBackend {
public:
    GpioPwmBackend(int pin, int pwm_range, int max_brightness)
        : pin_(pin), pwm_range_(pwm_range), max_brightness_(max_brightness), last_value_(0) {
        if (wiringPiSetupGpio() != 0) {
            throw std::runtime_error("Failed to initialize GPIO");
        }
        if (softPwmCreate(pin_, 0, pwm_range_) != 0) {
            throw std::runtime_error("Failed to initialize soft PWM");
        }
    }

    int get_current() const override {
        return last_value_;
    }

    void set(int value) override {
        int clamped = std::max(0, std::min(value, max_brightness_));
        int pwm_value = static_cast<int>(
            (static_cast<long long>(clamped) * pwm_range_) / max_brightness_);
        if (pwm_value < 0) pwm_value = 0;
        if (pwm_value > pwm_range_) pwm_value = pwm_range_;
        softPwmWrite(pin_, pwm_value);
        last_value_ = clamped;
    }

private:
    int pin_;
    int pwm_range_;
    int max_brightness_;
    int last_value_;
};

} // namespace

std::unique_ptr<BrightnessBackend> createBrightnessBackend(const Config& cfg) {
    if (cfg.backend == "gpio") {
        return std::unique_ptr<BrightnessBackend>(
            new GpioPwmBackend(cfg.gpio_pin, cfg.pwm_range, cfg.max_brightness));
    }
    return std::unique_ptr<BrightnessBackend>(new SysfsBackend(cfg.brightness_path));
}

#pragma once
#include <memory>

struct Config;

class BrightnessBackend {
public:
    virtual ~BrightnessBackend() = default;
    virtual int get_current() const = 0;
    virtual void set(int value) = 0;
};

std::unique_ptr<BrightnessBackend> createBrightnessBackend(const Config& cfg);

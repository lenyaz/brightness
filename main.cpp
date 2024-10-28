#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>
#include <cmath>

const std::string BRIGHTNESS_PATH = "/sys/class/backlight/10-0045/brightness";

// Function to read the current brightness
int get_current_brightness() {
    std::ifstream brightness_file(BRIGHTNESS_PATH);
    int brightness = 0;
    if (brightness_file.is_open()) {
        brightness_file >> brightness;
        brightness_file.close();
    } else {
        std::cerr << "Error: Unable to read brightness!" << std::endl;
    }
    return brightness;
}

// Function to set the brightness
void set_brightness(int value) {
    std::ofstream brightness_file(BRIGHTNESS_PATH);
    if (brightness_file.is_open()) {
        brightness_file << value;
        brightness_file.close();
    } else {
        std::cerr << "Error: Unable to set brightness!" << std::endl;
    }
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <target_brightness> <transition_time_ms>" << std::endl;
        return 1;
    }

    int target_brightness = std::stoi(argv[1]);
    int transition_time_ms = std::stoi(argv[2]);

    if (target_brightness < 0 || target_brightness > 255) {
        std::cerr << "Error: Brightness should be between 0 and 255." << std::endl;
        return 1;
    }

/*
    switch (argv[1]) {
        case "max":
		target_brightness=255; break;
	case "min":
		target_brightness=0;   break;
    }
*/
    int current_brightness = get_current_brightness();
    int steps = 255;
    double sleep_time_ms = static_cast<double>(transition_time_ms) / steps;
    double brightness_step = (target_brightness - current_brightness) / static_cast<double>(steps);

    for (int i = 0; i <= steps; ++i) {
        int new_brightness = std::round(current_brightness + i * brightness_step);
        set_brightness(new_brightness);
        std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time_ms)));
    }

    // Ensure final brightness is set correctly
    set_brightness(target_brightness);

    return 0;
}

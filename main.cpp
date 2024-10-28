#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>

const std::string BRIGHTNESS_PATH = "/sys/class/backlight/10-0045/brightness";

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


//Usage: ./brightness <target_brightness> <transition_time_ms> (e.g../brightness 255 1000)
int main(int argc, char* argv[]) { 
	
	//DEBUG
	std::cout << "   [DEBUG]   argv[1]=" << argv[1] << std::endl;
	std::cout << "   [DEBUG]   argv[2]=" << argv[2] << std::endl;

	if (argc == 2) int transition_time_ms = 0;

	else {
		if (argc > 3) {
			std::cerr << "Usage: " << argv[0] << " <target_brightness> <transition_time_ms>" << std::endl;
			return 1;
		}
	}

	int target_brightness = 0;
	int transition_time_ms = 0;
	bool argument_flag = false;

	std::string argument_target_brightness(argv[1]);


	//fuck dem switch case statements with strings in cpp
	if (argument_target_brightness == "max") { target_brightness = 255; argument_flag = true; } else { std::cout << "   [DEBUG]   not max" <<std::endl; }
	if (argument_target_brightness == "min") { target_brightness = 1; argument_flag = true; } else { std::cout << "   [DEBUG]   not min" << std::endl; }
	if (argument_target_brightness == "off") { target_brightness = 0; argument_flag = true; } else { std::cout << "   [DEBUG]   not off" << std::endl; }

	std::cout << "   [DEBUG]   argument_flag=" << argument_flag << std::endl;

	if (argument_flag == false) { target_brightness = std::stoi(argv[1]); } //TODO: try to convert to number, if fail - crashme
	
	transition_time_ms = std::stoi(argv[2]);

	if (target_brightness < 0 || target_brightness > 255) {
		std::cerr << "Error: Brightness should be between 0 and 255." << std::endl;
		return 1;
	}

	int current_brightness = get_current_brightness();

	int steps = abs(current_brightness - target_brightness); //TODO: needs rework to optimize // abs(255 - 0) = 255, abs(128 - 255) = 127 

	double sleep_time_ms = static_cast<double>(transition_time_ms) / steps; //time per step = transition_time_ms / steps
	double brightness_step = (target_brightness - current_brightness) / static_cast<double>(steps); //brightness step = (target_brightness - current_brightness) / steps          255 - 0 / 1000 = 0.255


	for (int i = 0; i <= steps; ++i) {
		int new_brightness = std::round(current_brightness + i * brightness_step);
		set_brightness(new_brightness);
		std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(sleep_time_ms)));
	}

	// Ensure final brightness is set correctly
	set_brightness(target_brightness);

	return 0;
}

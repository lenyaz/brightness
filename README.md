# Brightness Control for Raspberry Pi MIPI-connected Display ☀️

A somewhat quick-and-dirty automatic/manual display brightness solution for Raspberry Pi. Works with VEML7700 ambient light sensors and displays that cooperate with sysfs.

## Feature Overview

This project offers next brightness controls:
- **Auto-Brightness Mode** ```./brightness --auto``` - adjusts based on ambient light
- **Manual Control** ```./brightness <0-255>``` - for when you want to take charge
- **Lux Monitoring** ```./brightness --showlux``` - troubleshooting or just because data is fun
- **Systemd Preset Integration** ```systemctl enable brightness-auto.service``` - runs reliably in the background

## Hardware Requirements 

- Raspberry Pi (tested on Pi 4) 
- VEML7700 sensor (I2C-connected) 
- Display with sysfs backlight control (tested on [noname 7 inch 800x480 Aliexpress display with touch controls](https://www.aliexpress.us/item/3256805938513955.html))

## Software Requirements

- [WiringPi library](https://github.com/WiringPi/WiringPi)

## Installation ⚡

1. Clone repository:
   ```bash
   git clone https://github.com/lenyaz/brightness.git
   cd brightness
   ```

2. Build project 🤞:
   ```bash
   make
   ```
>**Hint**: you can install and enable systemd service using this line:
>```bash
>make install
>```

3. Optional systemd setup (```./add-systemd-service.sh```):
   ```bash
   sudo cp brightness-auto.service /etc/systemd/system/
   sudo systemctl daemon-reload
   sudo systemctl enable brightness-auto.service
   sudo systemctl start brightness-auto.service
   ```

## Usage 

### Manual Brightness Control
```bash
./brightness 128  # middle of 0-255 range
./brightness max  # maximum brightness possible (255)
./brightness min  # minimum brightness possible (1)
./brightness off  # display off (0)
```

### Automatic Mode
```bash
./brightness --auto  # "Ambient Light: 20.22 lux, Setting brightness to: 6" 
```

### Monitoring Mode
```bash
./brightness --showlux  # "Ambient Light: 20.00 lux"
```

## TODO List 📝

- **Optimization**: Make it less memory and CPU hungry (types optimization etc.)
- **Direct display brightness via GPIO**: For speed (wiringpi?)
- **Config file**: Stop hardcoding like it's 1979
- **Error handling**: Because shit happens

Licensed under [BSD 3-Clause License](LICENSE), Leanid "lenyaz" Veraksa, 2025
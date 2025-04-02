# Define the number of processors
NPROC = $(shell nproc)

# Define the compiler and flags
CXX = g++
WIRINGPI = -lwiringPi
CXXFLAGS += -Wall -Wextra

.PHONY: all o3 test clean test-auto test-manual install uninstall

all: brightness

o3: CXXFLAGS += -O3
o3: brightness

brightness: brightness.cpp veml7700.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(WIRINGPI)

test-auto: all
	./brightness --auto && sleep 2s && ./brightness 255 1000

test-manual: all
	./brightness 0 16000 && sleep 2s && ./brightness 255 16000

test: test-auto test-manual

clean:
	rm -f brightness
	rm -f /opt/brightness/brightness

install: brightness
	mkdir -p /opt/brightness
	cp ./brightness /opt/brightness/brightness
	chmod +x /opt/brightness/brightness
	./add-systemd-service.sh

uninstall: clean

# Run make with the maximum number of processes
.DEFAULT_GOAL := all
MAKEFLAGS += -j$(NPROC)

debug: CXXFLAGS += -g
debug: brightness
# Define the number of processors
NPROC = $(shell nproc)

# Define the compiler and flags
CXX = g++
WIRINGPI = -lwiringPi

.PHONY: all o3 test clean

all: brightness veml7700

o3: CXXFLAGS += -O3
o3: brightness veml7700

brightness: brightness.cpp veml7700.cpp
	$(CXX) $(CXXFLAGS) -o $@ $^ $(WIRINGPI)

veml7700: veml7700.cpp
	$(CXX) $(CXXFLAGS) -o $@ $< $(WIRINGPI)

test: all
	./brightness 0 16000 && sleep 2s && ./brightness 255 16000

clean:
	rm -f brightness veml7700

# Run make with the maximum number of processes
.DEFAULT_GOAL := all
MAKEFLAGS += -j$(NPROC)

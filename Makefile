all:
	g++ -o brightness main.cpp

o3:
	g++ -O3 -o brightness main.cpp

test: all
	./brightness 0 16000 && sleep 2s && ./brightness 255 16000

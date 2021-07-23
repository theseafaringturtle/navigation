CPP = g++
.PHONY: all
all: LSM9DS1_Reader

LSM9DS1_Reader: LSM9DS1_reader.cpp
	$(CPP) -g -DDEBUG -Wall -O2  *.cpp *.h -o LSM9DS1_Reader -llsm9ds1 -lwiringPi -lpthread -std=c++17

clean:
	rm -f LSM9DS1_Reader


CPP = g++
.PHONY: all
all: LSM9DS1_Sensor
CPP_FILES = main.cpp LSM9DS1_sensor.cpp LSM9DS1_sensor.h LSM9DS1_utils.h LSM9DS1_utils.cpp sensor.hpp

LSM9DS1_Sensor:
	$(CPP) -J4 -g -DDEBUG $(CPP_FILES) -Wall -O2 -Wl,-rpath=./library -L./library comm/comm_data.h -o LSM9DS1_client -llsm9ds1 -lrt -lpthread -std=c++17

clean:
	rm -f LSM9DS1_Sensor

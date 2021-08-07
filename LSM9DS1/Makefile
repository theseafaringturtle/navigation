CPP = g++
.PHONY: all clean
all: LSM9DS1_Sensor
CPP_FILES = main.cpp LSM9DS1_sensor.cpp LSM9DS1_sensor.h LSM9DS1_utils.h LSM9DS1_utils.cpp ../comm/sensor.hpp ../comm/imu_comm_data.h

LSM9DS1_sensor.o: LSM9DS1_sensor.cpp LSM9DS1_sensor.h
LSM9DS1_utils.o: LSM9DS1_utils.cpp LSM9DS1_utils.h
main.o: main.cpp

LSM9DS1_Sensor: LSM9DS1_sensor.o LSM9DS1_utils.o main.o
	$(CPP) -g -DDEBUG LSM9DS1_sensor.o LSM9DS1_utils.o main.o -Wall -O2 -Wl,-rpath=./library -L./library  -o LSM9DS1_client -llsm9ds1 -lrt -lpthread -std=c++17

clean:
	rm -f LSM9DS1_Sensor

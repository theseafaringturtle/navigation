CPP = g++
.PHONY: all
all: consumer
CPP_FILES=consumer.cpp comm/socket_setup.hpp comm/imu_comm_data.h comm/encoders_comm_data.h

clean: 
	rm consumer

consumer: 
	$(CPP) -J4 -g -DDEBUG -Wall -O2 $(CPP_FILES) -o consumer -std=c++17 -lrt -lpthread

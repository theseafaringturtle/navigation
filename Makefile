CPP = g++
.PHONY: all clean
all: consumer
CPP_FILES=consumer.cpp comm/socket_setup.hpp comm/imu_comm_data.h comm/encoders_comm_data.h

consumer.o: $(CPP_FILES)

clean: 
	rm consumer

consumer: consumer.o
	$(CPP) -g -DDEBUG -Wall -O2 consumer.o -o consumer -std=c++17 -lrt -lpthread

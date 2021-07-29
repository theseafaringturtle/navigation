CPP = g++
.PHONY: all
all: consumer

clean: 
	rm consumer

consumer: 
	$(CPP) -g -DDEBUG -Wall -O2 consumer.cpp LSM9DS1/comm/comm_data.h -o consumer -std=c++17


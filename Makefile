CPP = g++
.PHONY: all
all: consumer

clean: 
	rm consumer; rm rotary;

consumer: 
	$(CPP) -g -DDEBUG -Wall -O2 consumer.cpp LSM9DS1/comm/socket_setup.hpp LSM9DS1/comm/comm_data.h -o consumer -std=c++17 -lrt -lpthread

rotary:
	$(CPP) -g -DDEBUG -Wall -O2 consumer.cpp consumer.hpp -o rotary -std=c++17
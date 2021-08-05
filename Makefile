CPP = g++
.PHONY: all
all: consumer
CPP_FILES=consumer.cpp Encoder/encoder.hpp comm/socket_setup.hpp comm/comm_data.h

clean: 
	rm consumer

consumer: 
	$(CPP) -g -DDEBUG -Wall -O2 $(CPP_FILES) -o consumer -std=c++17 -lrt -lpthread

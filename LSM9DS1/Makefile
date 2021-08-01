CPP = g++
.PHONY: all
all: LSM9DS1_Reader

LSM9DS1_Reader: main.cpp
	$(CPP) -J4 -g -DDEBUG -Wall -O2 -Wl,-rpath=./library -L./library LSM9DS1_reader.cpp comm/comm.hpp comm/comm_data.h -o LSM9DS1_client -llsm9ds1 -lrt -lpthread -std=c++17

clean:
	rm -f LSM9DS1_Reader

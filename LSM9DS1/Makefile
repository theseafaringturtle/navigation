CPP = g++
.PHONY: all
all: LSM9DS1_Reader

LSM9DS1_Reader: main.cpp
	$(CPP) -g -DDEBUG -Wall -O2 LSM9DS1_reader.cpp comm/comm.cpp comm/comm.h comm/producer.cpp  -o LSM9DS1_client -llsm9ds1 -lrt -lpthread -std=c++17

clean:
	rm -f LSM9DS1_Reader


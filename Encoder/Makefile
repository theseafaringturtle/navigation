CPP = g++
.PHONY: all
all: Encoder_Sensor
CPP_FILES = main.cpp dual_encoder.h dual_encoder.cpp ../comm/sensor.hpp ../comm/encoders_comm_data.h

Encoder_Sensor:
	$(CPP) -J4 -g -DDEBUG $(CPP_FILES) -Wall -O2 -o Encoder_Sensor -lrt -lpthread -std=c++17

clean:
	rm -f Encoder_Sensor


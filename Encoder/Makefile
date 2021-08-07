CPP = g++
.PHONY: all clean
all: Encoder_Sensor
CPP_FILES = main.cpp dual_encoder.h dual_encoder.cpp ../comm/sensor.hpp ../comm/encoders_comm_data.h

dual_encoder.o: dual_encoder.cpp dual_encoder.h ../comm/encoders_comm_data.h
main.o: main.cpp
OBJECTS=dual_encoder.o main.o

Encoder_Sensor: $(OBJECTS)
	$(CPP) -g -DDEBUG $(OBJECTS) -Wall -o Encoder_Sensor -lrt -lpthread -std=c++17

clean:
	rm -f Encoder_Sensor

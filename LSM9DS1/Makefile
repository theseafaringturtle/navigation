CC=g++
CFLAGS=-c -Wall -O2 -std=c++11
LDFLAGS=-Wl,-rpath=./library -L./library 
EXECUTABLE_NAME=LSM9DS1_Sensor

SRC=.
BIN=bin
OBJ=$(BIN)/obj

LIBRARIES=-lrt -lpthread -llsm9ds1 

SOURCE_FILES=\
    main.cpp \
    LSM9DS1_sensor.cpp \
	LSM9DS1_utils.cpp

EXECUTABLE_FILE = $(EXECUTABLE_NAME:%=$(BIN)/%)
OBJECT_FILES    = $(SOURCE_FILES:%.cpp=$(OBJ)/%.o)

build: $(EXECUTABLE_FILE)

clean:
	rm -rf $(BIN)

.PHONY: build clean

$(EXECUTABLE_FILE): $(OBJECT_FILES)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIBRARIES)

$(OBJECT_FILES): $(OBJ)/%.o: %.cpp
	mkdir -p $(@D)
	$(CC) $(CFLAGS) -o $@ $< 

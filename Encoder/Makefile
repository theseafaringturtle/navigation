CC=g++
CFLAGS=-c -Wall -O2 -g -std=c++11
LDFLAGS=
EXECUTABLE_NAME=Encoder_Sensor

SRC=.
BIN=bin
OBJ=$(BIN)/obj

LIBRARIES=-lrt -lpthread

SOURCE_FILES=\
    main.cpp \
    dual_encoder.cpp

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

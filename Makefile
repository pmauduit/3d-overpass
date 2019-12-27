#!/usr/bin/make -f

BIN:=3d-overpass
BIN_DEP:=overpass-reader.cpp ogr-parser.cpp
BIN_OBJ:=$(BIN_DEP:%.cpp=%.o)

INSTALL_DIR:=./$(BIN)

CC=$(shell which g++)
GDB=$(shell which gdb)
VALGRIND=$(shell which valgrind)
STRIP=$(shell which strip)

CFLAGS=-Wall $(shell pkg-config --cflags gdal openscenegraph) -g
LDFLAGS=$(shell pkg-config --libs gdal openscenegraph) -lcurl

.PHONY: clean

all: $(BIN)

%.o: %.c
	$(CC) -c $< -o $@ $(CFLAGS) $(LDFLAGS)

$(BIN): $(BIN_OBJ)
	$(CC) -o $(BIN) $(BIN_DEP) $(CFLAGS) $(LDFLAGS)
clean:
	rm -f $(BIN) $(BIN_OBJ)

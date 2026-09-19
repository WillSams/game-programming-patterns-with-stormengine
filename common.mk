#!/bin/sh

BIN	= $(NAME)
# Repo root, derived from this file's own location, so patterns work at any
# folder depth (e.g. patterns/<name>/ or patterns/<category>/<name>/).
ROOT_DIR  		= $(abspath $(dir $(lastword $(MAKEFILE_LIST))))
BIN_DIR   		= $(ROOT_DIR)/bin
TARGET 			= $(BIN_DIR)/$(BIN)
TESTTARGET 		= $(BIN_DIR)/test-$(BIN)
DATA_PREFIX   	= $(ROOT_DIR)/assets/

# ── THE ENGINE IS THE PINNED SUBMODULE, ALWAYS ─────────────────────────────
#
# This repo tracks Storm Engine v2 and builds against its own submodule, at the
# revision pinned in external/storm-engine-v2. There is deliberately no
# "use the system install if the submodule is missing" branch: the whole reason
# the submodule exists is that a machine's system install can be a different
# engine than CI's, and a fallback is exactly how the two quietly diverge.
# Measured before this change: 2.3.0 installed here, 2.3.1 in CI.
#
# `engine.mk` owns building it; every target here depends on the library, so the
# first `make` in any pattern builds the engine once and the rest reuse it.
include $(ROOT_DIR)/engine.mk

LIB     = -L$(ENGINE_BIN) -Wl,-rpath,$(ENGINE_BIN) \
	-lstormenginev2 \
	-lSDL2 -lSDL2_image -lSDL2_ttf -lSDL2_mixer
# include/ holds this repo's engine bridge; the shim resolves <stormengine2/...>
# to the pinned engine's own headers.
INCLUDE = -I$(ROOT_DIR)/include -I$(ENGINE_INC)

CC = g++
CCFLAGS = -Wall -c -g -std=c++17 -DDATA_PREFIX=\"$(DATA_PREFIX)\" \
	-include engineGlobal.h \
	-Wno-reorder -Wno-unused-parameter -Wno-unused-variable -Wno-unused-function  $(INCLUDE) 

SRCS	= $(wildcard src/**/*.cpp)
SRCS	+= $(wildcard src/*.cpp)
SRCS	+= ./main.cpp
OBJS 	= $(SRCS:.cpp=.o)

all: clean  $(TARGET)

clean:
	rm -f $(BIN_DIR)/* && rm -f $(shell find . -name "*.o")

run:
	$(TARGET)
	
test: test-target

.cpp.o: 
	$(CC) $(CCFLAGS) $< -o $@

$(TARGET) : $(OBJS) $(ENGINE_LIB)
	mkdir -p $(BIN_DIR)
	$(CC) $^ $(LIB) -o $@
	
memcheck:
	valgrind --log-file=valgrind.output --leak-check=yes --tool=memcheck $(TARGET)

TESTRCS  = $(wildcard specs/*.cpp)
TESTRCS  += $(wildcard src/*.cpp)
TESTRCS  += $(wildcard src/**/*.cpp)
TESTOBJS  = $(TESTRCS:.cpp=.o)
	
test: test-target

run-test:
	$(TESTTARGET)

test-target: $(TESTOBJS) $(ENGINE_LIB)
	mkdir -p $(BIN_DIR)
	$(CC) $^ $(LIB) -pthread -o $(TESTTARGET)

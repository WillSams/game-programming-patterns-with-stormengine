#!/bin/sh

# ⚠️ THE DEFAULT GOAL IS STATED, NOT INHERITED. `engine.mk` is included from
# here and its first target is the engine library, which silently became the
# default goal -- so a bare `make` built the ENGINE and never the example, for
# every pattern, and the README says `make` builds the example. A default goal
# that depends on include order is the kind of thing that breaks in one commit
# and is noticed three commits later; name it.
.DEFAULT_GOAL := all

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

# ⚠️ `all` USED TO DEPEND ON `clean`, AND `clean` WIPED THE WHOLE SHARED
# `$(BIN_DIR)`. Every pattern links into the one repo-root bin/, so building any
# demo deleted every other demo's binary -- `make` in command, then `make run` in
# flyweight, met "No such file", and `bin/` held one pattern's output no matter how
# many existed. It also forced a full rebuild on every invocation. Scoped to this
# pattern's own outputs, a rebuild is incremental and `bin/` accumulates.
# The root Makefile keeps the wipe-everything escape hatch.
all: $(TARGET)

clean:
	rm -f $(TARGET) $(TESTTARGET) $(OBJS) $(TESTOBJS)

run:
	$(TARGET)
	
# NOTE: `test: test-target` used to be declared twice in this file (here and
# below run-test, where it belonged). Harmless, but it read as two different
# rules doing two different things. One copy, next to test-target.

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

# ⚠️ run-test DEPENDS ON THE BUILD. It used to just execute $(TESTTARGET), so it
# only worked when something else had already built it -- CI runs `make test`
# first and therefore never saw the gap, while a reader following the README
# ("make run-test") got "Command not found". Worse while `make` still cleaned
# first: the binary it was about to run had just been deleted.
run-test: test-target
	$(TESTTARGET)

test-target: $(TESTOBJS) $(ENGINE_LIB)
	mkdir -p $(BIN_DIR)
	$(CC) $^ $(LIB) -pthread -o $(TESTTARGET)

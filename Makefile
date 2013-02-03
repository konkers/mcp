DIRS = lua

all: dirs mcp


DUNAME := $(shell uname -s)
ARCH := $(shell uname -m)

ifeq ($(UNAME),Darwin)
CFLAGS += -I/opt/local/include
LDFLAGS += -L/opt/local/lib
endif
ifeq ($(UNAME),Linux)
CFLAGS +=
LDFLAGS += 
endif
CFLAGS += -DUSE_LUA
CXXFLAGS = $(CFLAGS) -std=c++11
LDXXFLAGS = $(LDFLAGS)

LDLIBS := -lusb-1.0

OBJS = main.o Dongle.o Ds18b20.o Pid.o Thread.o WebServer.o mongoose.o

TARGETS=mcp

mcp: $(OBJS)
	$(LINKXX) #$(LDXXFLAGS) -o $@ $^ $(LDLIBS)

include rules.mk

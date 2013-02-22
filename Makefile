DIRS = lua avr

all: dirs mcp test


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

MCP_OBJS = main.o \
	mongoose.o \
	Dongle.o \
	DongleThread.o \
	Ds18b20.o \
	EventQueue.o \
	Pid.o \
	Thread.o \
	TimerThread.o \
	WebServer.o

TEST_OBJS = test.o \
	Dongle.o \
	Ds18b20.o

OBJS=${MCP_OBJS} ${TEST_OBJS}

TARGETS=mcp test

mcp: $(MCP_OBJS)
	$(LINKXX) #$(LDXXFLAGS) -o $@ $^ $(LDLIBS)

test: $(TEST_OBJS)
	$(LINKXX) #$(LDXXFLAGS) -o $@ $^ $(LDLIBS)

include rules.mk

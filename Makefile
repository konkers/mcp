UNAME := $(shell uname -s)
ARCH := $(shell uname -m)

ifeq ($(UNAME),Darwin)
CFLAGS += -I/opt/local/include
LDFLAGS += -L/opt/local/lib
endif
ifeq ($(UNAME),Linux)
CFLAGS +=
LDFLAGS += 
endif
CXXFLAGS = $(CFLAGS) #-std=c++11
LDXXFLAGS = $(LDFLAGS)

LDLIBS := -lusb-1.0

OBJS = main.o Dongle.o Ds18b20.o Pid.o

mcp: $(OBJS)
	$(CXX) $(LDXXFLAGS) -o $@ $^ $(LDLIBS)

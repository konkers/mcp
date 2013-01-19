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
CXXFLAGS = $(CFLAGS)
LDXXFLAGS = $(LDFLAGS)

LDLIBS := -lusb-1.0

OBJS = main.o Dongle.o

mcp: $(OBJS)
	$(CXX) $(LDXXFLAGS) -o $@ $^ $(LDLIBS)

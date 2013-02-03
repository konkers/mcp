OPT = -g -O2
CFLAGS += ${OPT} -Wall -Werror -DUSE_LUA -I${MCP_DIR}/lua
CXXFLAGS += ${CFLAGS}
LIBS += -lusb-1.0 -llua
LIBSXX += ${LIBS} 
LDFLAGS += 
LDXXFLAGS += ${LDFLAGS} -L${MCP_DIR}/lua 
RANLIB = ranlib

ifeq ("${SYSTEM}","Linux")
CFLAGS += 
LIBS += -ldl -lpthread
endif

ifeq ("${SYSTEM}","Darwin")
CFLAGS += -I/opt/local/include -DOSX
LDFLAGS += -L/opt/local/lib
endif

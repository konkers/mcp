$(call build-mk-start)

HOST_OPT = -g -O2
HOST_CFLAGS += ${OPT} -Wall -Werror
HOST_CXXFLAGS += ${CFLAGS} -std=c++11 -stdlib=libc++
HOST_LIBS +=
HOST_LIBSXX += ${LIBS}
HOST_LDFLAGS += -L${BUILD_TOP} -L${OUT}/lib
HOST_LDXXFLAGS += ${HOST_LDFLAGS}
HOST_RANLIB = ranlib

ifeq ("${UNAME}","Linux")
HOST_CFLAGS +=
HOST_LIBS += -ldl -lpthread
endif

ifeq ("${UNAME}","Darwin")
HOST_CFLAGS += -I/opt/local/include -DOSX
HOST_LDFLAGS += -L/opt/local/lib
HOST_CXXFLAGS += -I/opt/local/include -DOSX
HOST_LDXXFLAGS += -L/opt/local/lib
endif

OUT_HOST_OBJ := $(OUT)/host_obj

BUILD_EXECUTABLE := $(BUILD_TOP)/build/executable.mk
BUILD_STATIC_LIB := $(BUILD_TOP)/build/static_lib.mk
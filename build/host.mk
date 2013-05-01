## Copyright 2013 Erik Gilling <konkers@konkers.net>
##
## Licensed under the Apache License, Version 2.0 (the "License");
## you may not use this file except in compliance with the License.
## You may obtain a copy of the License at
##
##     http://www.apache.org/licenses/LICENSE-2.0
##
## Unless required by applicable law or agreed to in writing, software
## distributed under the License is distributed on an "AS IS" BASIS,
## WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
## See the License for the specific language governing permissions and
## limitations under the License.
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
HOST_SO_LDFLAGS += -shared -fPIC
endif

ifeq ("${UNAME}","Darwin")
HOST_CFLAGS += -I/opt/local/include -DOSX
HOST_LDFLAGS += -L/opt/local/lib
HOST_CXXFLAGS += -I/opt/local/include -DOSX
HOST_LDXXFLAGS += -L/opt/local/lib
HOST_SO_LDFLAGS += -bundle -undefined dynamic_lookup
endif

OUT_HOST_OBJ := $(OUT)/host_obj

BUILD_EXECUTABLE := $(BUILD_TOP)/build/executable.mk
BUILD_STATIC_LIB := $(BUILD_TOP)/build/static_lib.mk
BUILD_PLUGIN := $(BUILD_TOP)/build/plugin.mk
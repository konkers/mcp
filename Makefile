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
## limitations under the Licene.

CLEAN_MAKEFILE_LIST := $(MAKEFILE_LIST)
-include $(BUILD_TOP)/local.mk

all:

include build/build.mk
MAKEFILE_LIST := $(CLEAN_MAKEFILE_LIST)

QUIET ?= @

include $(BUILD_TOP)/build/host.mk
include $(BUILD_TOP)/build/avr.mk

M_NAME := libmcp
M_SRCS := \
	DongleThread.cpp \
	Ds18b20.cpp \
	EventQueue.cpp \
	OwIO.cpp \
	Pid.cpp \
	SimDongle.cpp \
	State.cpp \
	Thread.cpp \
	TimerThread.cpp \
	UsbDongle.cpp \
	WebServer.cpp
include $(BUILD_STATIC_LIB)

M_NAME := mcp
M_SRCS := main.cpp
M_LDFLAGS := -lusb-1.0
M_LIBS := liblua libmongoose libmcp
include $(BUILD_EXECUTABLE)

include $(call subdir-mkfiles)

.PHONY: all
all: $(TARGETS)
	@echo $*

-include $(DEPS)

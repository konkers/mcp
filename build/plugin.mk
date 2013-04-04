## Copyright 2013 Erik Gilling <konkers@konkers.net>
## Copyright 2011 Brian Swetland <swetland@frotz.net>
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

M_NAME := $(strip $(M_NAME))

TARGETS := $(TARGETS) module_$(M_NAME)

# sanity check
ifeq "$(M_NAME)" ""
$(error No module name specified)
endif

ifeq ($(M_LIB_HEADER_DIR),)
M_LIB_HEADER_DIR := $(call my-dir)
endif
HOST_INCS := $(HOST_INCS) -I$(M_LIB_HEADER_DIR)


include $(BUILD_TOP)/build/host-common.mk

$(OUT)/lib/$(M_NAME).so: _OBJS := $(M_OBJS)
$(OUT)/lib/$(M_NAME).so: _LIBS := $(M_LIBS:lib%=-l%)
$(OUT)/lib/$(M_NAME).so: _LDFLAGS := $(M_LDFLAGS)
ifneq ($(filter %.cpp, $(M_SRCS)),)
$(OUT)/lib/$(M_NAME).so: $(M_OBJS)
	@echo "  LINK++ " $@ $(M_CXX)
	@$(MKDIR)
	$(QUIET)$(CXX) $(HOST_CXXFLAGS) $(HOST_SO_LDFLAGS) $(HOST_LDXXFLAGS) \
	  $(HOST_LIBSXX) $(_LDFLAGS) -o $@ $(_OBJS) $(_LIBS)
else
$(OUT)/lib/$(M_NAME).so: $(M_OBJS)
	@echo "  LINK   " $@
	@$(MKDIR)
	$(QUIET)$(CC) $(HOST_CFLAGS) $(HOST_SO_LDFLAGS) $(HOST_LDFLAGS) \
	  $(HOST_LIBS) $(_LDFLAGS) -o $@ $(_OBJS) $(_LIBS)
endif

module_$(M_NAME): $(OUT)/lib/$(M_NAME).so

$(info module $(M_NAME))

include $(BUILD_TOP)/build/clean-vars.mk
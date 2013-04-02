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

M_OBJS := $(M_SRCS:.c=.o)
M_OBJS := $(M_OBJS:.cpp=.o)
M_INCS := $(M_INCS) $(M_LIBS:%=-I$(OUT_HOST_OBJ)/%/includes)
M_DEPS := $(M_LIBS:%=module_%)

M_OBJS := $(addprefix $(OUT_HOST_OBJ)/$(M_NAME)/,$(M_OBJS))
DEPS := $(DEPS) $(M_OBJS:%o=%d)

include $(BUILD_TOP)/build/host-common.mk
$(OUT)/$(M_NAME): _OBJS := $(M_OBJS)
$(OUT)/$(M_NAME): _LIBS := $(M_LIBS:lib%=-l%)
$(OUT)/$(M_NAME): _LDFLAGS := $(M_LDFLAGS)
ifneq ($(filter %.cpp, $(M_SRCS)),)
$(OUT)/$(M_NAME): $(M_OBJS)
	@echo "  LINK++ " $@ $(M_CXX)
	$(QUIET)$(CXX) $(HOST_CXXFLAGS) $(HOST_LDXXFLAGS) $(HOST_LIBSXX) $(_LDFLAGS) -o $@ $(_OBJS) $(_LIBS)
else
$(OUT)/$(M_NAME): $(M_DEPS) $(M_OBJS)
	@echo "  LINK   " $@
	$(QUIET)$(CC) $(HOST_CFLAGS) $(HOST_LDFLAGS) $(HOST_LIBS) $(_LDFLAGS) -o $@ $(_OBJS) $(_LIBS)
endif

module_$(M_NAME): $(OUT)/$(M_NAME)

$(info module $(M_NAME))

include $(BUILD_TOP)/build/clean-vars.mk
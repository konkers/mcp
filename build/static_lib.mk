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
M_OBJS := $(addprefix $(OUT_HOST_OBJ)/$(M_NAME)/,$(M_OBJS))
M_HEADERS := $(addprefix $(OUT_HOST_OBJ)/$(M_NAME)/includes/,$(M_HEADERS))

DEPS := $(DEPS) $(M_OBJS:%o=%d)

include $(BUILD_TOP)/build/host-common.mk

$(OUT)/lib/$(M_NAME).a: _OBJS := $(M_OBJS)
$(OUT)/lib/$(M_NAME).a: _LIBS := $(M_LIBS)
$(OUT)/lib/$(M_NAME).a: $(M_OBJS) $(M_HEADERS)
	@$(MKDIR)
	@echo "  MKLIB  " $@
	$(QUIET)rm -f $@
	$(QUIET)$(AR) -cr $@ $^
	$(QUIET)$(HOST_RANLIB) $@

module_$(M_NAME): $(OUT)/lib/$(M_NAME).a

$(info module $(M_NAME))

include $(BUILD_TOP)/build/clean-vars.mk
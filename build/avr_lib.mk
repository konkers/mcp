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

M_NAME := $(strip $(M_NAME))

TARGETS := $(TARGETS) module_avr_$(M_NAME)

# sanity check
ifeq "$(M_NAME)" ""
$(error No module name specified)
endif

AVR_LIB_$(M_NAME)_SRCS := $(M_SRCS)
AVR_LIB_$(M_NAME)_DIR := $(call my-dir)

M_HEADERS := $(addprefix $(OUT_AVR_OBJ)/$(M_NAME)/includes/,$(M_HEADERS))
$(OUT_AVR_OBJ)/$(M_NAME)/includes/%: $(call my-dir)/%
	@$(MKDIR)
	@echo "  INC    " $<
	$(QUIET)cp $< $@

$(info avr_module $(M_NAME))

module_avr_$(M_NAME): $(M_HEADERS)

include $(BUILD_TOP)/build/clean-vars.mk
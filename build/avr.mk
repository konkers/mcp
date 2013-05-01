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

AVR_CC=avr-gcc
AVR_OBJCOPY=avr-objcopy
AVR_OBJDUMP=avr-objdump
AVR_SIZE=avr-size
AVR_AR=avr-ar
AVR_RANLIB=avr-ranlib

AVR_OPT = -g -O2
AVR_CFLAGS = ${OPT} -Wall -Werror

#CFLAGS=-mmcu=${CROSS_CPU} ${FOSCCFLAG} ${MCPMAINCFLAG} ${RS485ECHOCFLAG} 

OUT_AVR_OBJ := $(OUT)/avr_obj

define do-avr-lib
$(foreach src, $(AVR_LIB_$(1)_SRCS), \
$(eval \
M_OBJS := $(M_OBJS) $(OUT_AVR_OBJ)/$(M_NAME)/$(1)-$(src:%.c=%.o)\
);\
$(eval \
$(OUT_AVR_OBJ)/$(M_NAME)/$(1)-$(src:%.c=%.o): $(AVR_LIB_$(1)_DIR)/$(src)
	@$$(MKDIR)
	@echo "  AVRCC  " $$<
	$$(QUIET)$$(AVR_CC) $$(AVR_CFLAGS) $$(_INCS) -c $$< -o $$@ \
-MD -MT $$@ -MF $$(@:%o=%d)
)\
)
endef


BUILD_AVR_BIN := $(BUILD_TOP)/build/avr_bin.mk
BUILD_AVR_LIB := $(BUILD_TOP)/build/avr_lib.mk
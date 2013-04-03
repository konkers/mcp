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
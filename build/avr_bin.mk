$(call build-mk-start)

M_NAME := $(strip $(M_NAME))

TARGETS := $(TARGETS) module_avr_$(M_NAME)

# sanity check
ifeq "$(M_NAME)" ""
$(error No module name specified)
endif

AVR_CFLAGS := $(AVR_CFLAGS) -mmcu=$(M_CROSS_CPU)
ifneq ($(M_FOSC),)
AVR_CFLAGS := $(AVR_CFLAGS) -DFOSC=$(M_FOSC)
endif

M_OBJS := $(M_SRCS:.c=.o)
M_OBJS := $(addprefix $(OUT_AVR_OBJ)/$(M_NAME)/,$(M_OBJS))
M_INCS := $(M_INCS) $(M_LIBS:%=-I$(OUT_AVR_OBJ)/%/includes)
M_DEPS := $(M_LIBS:%=module_avr_%)

$(foreach lib, $(M_LIBS), $(call do-avr-lib,$(lib)))

$(M_OBJS): _INCS := $(M_INCS)

$(info $(M_OBJS))
$(OUT_AVR_OBJ)/$(M_NAME)/%.o: $(call my-dir)/%.c
	@$(MKDIR)
	@echo "  AVRCC   " $<
	$(QUIET)$(AVR_CC) $(AVR_CFLAGS) $(_INCS) -c $< -o $@ -MD -MT $@ -MF $(@:%o=%d)

$(OUT_AVR_OBJ)/$(M_NAME)/$(M_NAME).elf: _OBJS := $(M_OBJS)
$(OUT_AVR_OBJ)/$(M_NAME)/$(M_NAME).elf: _LDFLAGS := $(M_LDFLAGS)
$(OUT_AVR_OBJ)/$(M_NAME)/$(M_NAME).elf: $(M_OBJS)
	@echo " AVRLINK " $@
	$(QUIET)$(AVR_CC) $(AVR_CFLAGS) $(AVR_LDFLAGS) $(AVR_LIBS) \
		$(_LDFLAGS) -o $@ $(_OBJS) $(_LIBS)

$(OUT)/$(M_NAME).bin: $(OUT_AVR_OBJ)/$(M_NAME)/$(M_NAME).elf
	@echo "  AVROBJCPY " $@; ${AVR_OBJCOPY} -O binary $^ $@

module_avr_$(M_NAME): $(OUT)/$(M_NAME).bin

include $(BUILD_TOP)/build/clean-vars.mk
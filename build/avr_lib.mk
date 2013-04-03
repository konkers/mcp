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

$(info module $(M_NAME))

module_avr_$(M_NAME): $(M_HEADERS)

include $(BUILD_TOP)/build/clean-vars.mk
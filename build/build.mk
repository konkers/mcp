
BUILD_TOP := $(patsubst %/,%,$(dir $(firstword $(MAKEFILE_LIST))))
UNAME := $(shell uname)
UNAME_M := $(shell uname -m)

OUT := $(BUILD_TOP)/out

MKDIR = if [ ! -d $(dir $@) ]; then mkdir -p $(dir $@); fi
clean::
	@echo clean
	@rm -rf $(OUT)

define my-dir
$(strip $(patsubst %/,%,$(dir $(lastword $(MAKEFILE_LIST)))))
endef

define subdir-mkfiles
$(wildcard $(call my-dir)/*/module.mk)
endef

define build-mk-start
$(eval MAKEFILE_LIST := $(filter-out $(lastword $(MAKEFILE_LIST)), $(MAKEFILE_LIST)))
endef
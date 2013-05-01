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
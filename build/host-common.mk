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

M_OBJS := $(M_SRCS:.c=.o)
M_OBJS := $(M_OBJS:.cpp=.o)
M_OBJS := $(addprefix $(OUT_HOST_OBJ)/$(M_NAME)/,$(M_OBJS))
$(M_OBJS): _INCS := $(M_INCS)
$(M_OBJS): _CFLAGS := $(M_CFLAGS)

DEPS := $(DEPS) $(M_OBJS:%o=%d)

$(OUT_HOST_OBJ)/$(M_NAME)/%.o: $(call my-dir)/%.c
	@$(MKDIR)
	@echo "  CC     " $<
	$(QUIET)$(CC) $(HOST_CFLAGS) $(_CFLAGS) $(_INCS) $(HOST_INCS) -c $< -o $@ -MD -MT $@ -MF $(@:%o=%d)


$(OUT_HOST_OBJ)/$(M_NAME)/%.o: $(call my-dir)/%.cpp
	@$(MKDIR)
	@echo "  C++    " $<
	$(QUIET)$(CXX) $(HOST_CXXFLAGS) $(_CFLAGS) $(_INCS) $(HOST_INCS) -c $< -o $@ -MD -MT $@ -MF $(@:%o=%d)

$(OUT_HOST_OBJ)/$(M_NAME)/includes/%: $(call my-dir)/%
	@$(MKDIR)
	@echo "  INC    " $<
	$(QUIET)cp $< $@

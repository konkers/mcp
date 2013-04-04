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

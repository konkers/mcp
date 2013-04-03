$(call build-mk-start)

$(M_OBJS): _INCS := $(M_INCS)
$(M_OBJS): _CFLAGS := $(M_CFLAGS)
$(M_OBJS): $(M_LIBS:%=$(OUT)/lib/%.a) 

$(OUT_HOST_OBJ)/$(M_NAME)/%.o: $(call my-dir)/%.c
	@$(MKDIR)
	@echo "  CC     " $<
	$(QUIET)$(CC) $(HOST_CFLAGS) $(_CFLAGS) $(_INCS) -c $< -o $@ -MD -MT $@ -MF $(@:%o=%d)


$(OUT_HOST_OBJ)/$(M_NAME)/%.o: $(call my-dir)/%.cpp
	@$(MKDIR)
	@echo "  C++    " $<
	$(QUIET)$(CXX) $(HOST_CXXFLAGS) $(_CFLAGS) $(_INCS) -c $< -o $@ -MD -MT $@ -MF $(@:%o=%d)

$(OUT_HOST_OBJ)/$(M_NAME)/includes/%: $(call my-dir)/%
	@$(MKDIR)
	@echo "  INC    " $<
	$(QUIET)cp $< $@

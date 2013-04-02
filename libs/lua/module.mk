LUA_CORE_SRCS=	lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c \
	llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c \
	ltable.c ltm.c lundump.c lvm.c lzio.c
LUA_LIB_SRCS=	lauxlib.c lbaselib.c lbitlib.c lcorolib.c ldblib.c liolib.c \
	lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c

M_NAME := liblua
M_SRCS := $(LUA_CORE_SRCS) $(LUA_LIB_SRCS)
M_HEADERS := \
	lapi.h \
	lauxlib.h \
	lcode.h \
	lctype.h \
	ldebug.h \
	ldo.h \
	lfunc.h \
	lgc.h \
	llex.h \
	llimits.h \
	lmem.h \
	lobject.h \
	lopcodes.h \
	lparser.h \
	lstate.h \
	lstring.h \
	ltable.h \
	ltm.h \
	lua.h \
	luaconf.h \
	lualib.h \
	lundump.h \
	lvm.h \
	lzio.h \
	lua.hpp

include $(BUILD_STATIC_LIB)

LUA_CORE_SRCS=	lapi.c lcode.c lctype.c ldebug.c ldo.c ldump.c lfunc.c lgc.c \
	llex.c lmem.c lobject.c lopcodes.c lparser.c lstate.c lstring.c \
	ltable.c ltm.c lundump.c lvm.c lzio.c
LUA_LIB_SRCS=	lauxlib.c lbaselib.c lbitlib.c lcorolib.c ldblib.c liolib.c \
	lmathlib.c loslib.c ltablib.c lstrlib.c loadlib.c linit.c

M_NAME := liblua
M_SRCS := $(LUA_CORE_SRCS) $(LUA_LIB_SRCS)
M_LIB_HEADER_DIR := $(call my-dir)

ifeq ("${UNAME}","Linux")
M_CFLAGS := -DLUA_USE_LINUX
endif

ifeq ("${UNAME}","Darwin")
M_CFLAGS := -DLUA_USE_MACOSX
endif

include $(BUILD_STATIC_LIB)

M_NAME := libmongoose
M_SRCS := mongoose.c
M_CFLAGS := -DUSE_LUA
M_LIBS := liblua
include $(BUILD_STATIC_LIB)

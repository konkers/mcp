M_NAME := libavr
M_SRCS := owslave.c uart.c
M_HEADERS := owslave.h uart.h
include $(BUILD_AVR_LIB)

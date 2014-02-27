M_NAME := owrelay
M_CROSS_CPU := atmega88
M_FOSC := 18432000UL
M_CFLAGS := -DOW_TIMER=1
M_LIBS := libavr
M_SRCS := owrelay.c

include $(BUILD_AVR_BIN)

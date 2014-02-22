M_NAME := owdio
M_CROSS_CPU := atmega88
M_FOSC := 18432000UL
M_CFLAGS := -DOW_TIMER=1
M_LIBS := libavr
M_SRCS := owdio.c

include $(BUILD_AVR_BIN)

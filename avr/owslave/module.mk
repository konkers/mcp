M_NAME := owslave
M_CROSS_CPU := atmega2560
M_FOSC := 16000000UL
M_CFLAGS := -DOW_TIMER=5
M_LIBS := libavr
M_SRCS := owslave.c

include $(BUILD_AVR_BIN)

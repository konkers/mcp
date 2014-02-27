M_NAME := owuno
M_CROSS_CPU := atmega328p
M_FOSC := 16000000UL
M_CFLAGS := -DOW_TIMER=1
M_LIBS := libavr
M_SRCS := owuno.c

include $(BUILD_AVR_BIN)

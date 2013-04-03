



M_NAME := owslave
M_CROSS_CPU := atmega2560
M_FOSC := 16000000UL
M_LIBS := libavr
M_SRCS := owslave.c

#TARGET=owslave
#TARGETS=${TARGET}.elf ${TARGET}.bin

#ll: libavr.a ${TARGETS}

#include ${MCP_DIR}/rules.mk

# must be after include for LIBAVR_OBJS to work
#${TARGET}.elf: ${OBJS} libavr.a
#	${LINK}

include $(BUILD_AVR_BIN)
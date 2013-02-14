CROSS=avr
CROSS_CPU=atmega2560

FOSC=16000000UL

OBJS=owslave.o

TARGET=owslave
TARGETS=${TARGET}.elf ${TARGET}.bin

all: libavr.a ${TARGETS}

include ${MCP_DIR}/rules.mk

# must be after include for LIBAVR_OBJS to work
${TARGET}.elf: ${OBJS} libavr.a
	${LINK}

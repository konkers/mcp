
ifeq ("${CROSS_CPU}","")
CROSS_CPU=atmega88
endif
ifneq ("${FOSC}","")
FOSCCFLAG=-DFOSC=${FOSC}
endif

CC=avr-gcc
OBJCOPY=avr-objcopy
OBJDUMP=avr-objdump
SIZE=avr-size
AR=avr-ar
RANLIB=avr-ranlib

OPT=-O2 -g

CFLAGS=-mmcu=${CROSS_CPU} ${FOSCCFLAG} ${MCPMAINCFLAG} ${RS485ECHOCFLAG} \
	${OPT} -Wall -Werror \
	-I${MCP_DIR}/lib/c/include \
	-I${MCP_DIR}/avr/lib/include

LDFLAGS=-mmcu=${CROSS_CPU} -L.

LIBAVR_OBJS = libavr-uart.o #libavr-config.o libavr-mcpmain.o

LIBS=-lavr

libavr-%.o: ${MCP_DIR}/avr/lib/%.c
	@echo "  CC     " `basename $<`
	@${CC} -c ${CFLAGS} -o $@ $<


libmcp.a: ${LIBMCP_OBJS}
	${MKLIB}

libavr.a: ${LIBAVR_OBJS}
	${MKLIB}

%.bin: %.elf
	@echo "  OBJCPY " $@; ${OBJCOPY} -O binary $^ $@

%.lst: %.elf
	${OBJDUMP} --disassemble-all --source $^ > $@

ifeq ("${CROSS_CPU}","atmega48")
AVRDUDE_CPU=m48
else ifeq ("${CROSS_CPU}","atmega88")
AVRDUDE_CPU=m88p
else ifeq ("${CROSS_CPU}","atmega168")
AVRDUDE_CPU=m168
else ifeq ("${CROSS_CPU}","atmega2560")
AVRDUDE_CPU=m2560
else ifeq ("${CROSS_CPU}","atmega8")
AVRDUDE_CPU=m8
endif

flash: flash-${basename ${firstword ${filter %.bin, ${TARGETS}}}}

flash-%: %.bin
	avrdude -p ${AVRDUDE_CPU} -P ${AVRDEV} -c stk500v2 -b 115200 -U flash:w:$< ${FUSES}

clean-host:
	@rm -f ${LIBAVR_OBJS} libavr.a

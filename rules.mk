
ifeq ("${MCP_DIR}","")
MCP_DIR = ${shell pwd}
else
ifeq ("${SUBDIR}","")
SUBDIR := "${shell basename $$PWD}"
endif
endif

ifeq ("${SUBDIR}","")
NEWSUBDIR=""
else
NEWSUBDIR="${SUBDIR}/"
endif

SYSTEM=${shell uname}

ifeq ("${CROSS}","avr")
include ${MCP_DIR}/rules-avr.mk
else
include ${MCP_DIR}/rules-host.mk
endif


LINK =   @echo "  LINK   " $@; ${CC} ${LDFLAGS} -o $@ ${filter-out %.a, $^} ${LIBS}
LINKXX = echo "  LINK++ " $@; ${CXX} ${LDXXFLAGS} -o $@ ${filter-out %.a, $^} ${LIBSXX}

MKLIB=@echo "  MKLIB  " $@; rm -f $@; ${AR} -cr $@ $^; ${RANLIB} $@; echo

.PHONY: dirs clean clean-here clean-host dirname sizes sizes-here

dirname:
	@if [ "${TARGETS}" != "" ]; then \
		echo ;\
		echo "${SUBDIR}:";\
	fi

%.o: %.c
	@echo "  CC     " `basename $<`
	${CC} -c ${CFLAGS} -o $@ $<

%.s: %.c
	@echo "  CC -S  " `basename $<`
	${CC} -c ${CFLAGS} -S -o $@ $<

%.o: %.cpp
	@echo "  C++    " `basename $<`
	${CXX} -c ${CXXFLAGS} -o $@ $<

dirs:

	@for dir in ${DIRS}; do \
		make MCP_DIR=${MCP_DIR} SUBDIR="${NEWSUBDIR}$$dir" -C $$dir dirname all || exit 1;\
	done

clean: clean-here clean-host
	@echo "CLEAN  ${SUBDIR}"
	@rm -f ${OBJS} ${TARGETS}
	@for dir in ${DIRS}; do \
		make MCP_DIR=${MCP_DIR} SUBDIR="${NEWSUBDIR}$$dir" -C $$dir clean || exit 1;\
	done

sizes: sizes-here
	@for target in ${basename ${filter %.elf,${TARGETS}}}; do \
		printf "%-15s %6d %6d\n" \
			"$$target:" \
			`${SIZE} $$target.elf | tail -1 | awk '{print $$1 + $$2}'` \
			`${SIZE} $$target.elf | tail -1 | awk '{print $$2 + $$3}'`; \
	done
	@for dir in ${DIRS}; do \
		make MCP_DIR=${MCP_DIR} SUBDIR="${NEWSUBDIR}$$dir" -C $$dir sizes || exit 1;\
	done

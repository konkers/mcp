
#include "OwIO.hpp"

#define DS18B20_CMD_WRITE_SCRATCHPAD	0x4e
#define DS18B20_CMD_READ_SCRATCHPAD	0xbe

OwIO::OwIO(Dongle *dongle, Dongle::Addr addr) :
	dongle(dongle), addr(addr)
{
}


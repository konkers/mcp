
#include "Ds18b20.hpp"

#define DS18B20_CMD_CONVERT_T		0x44
#define DS18B20_CMD_WRITE_SCRATCHPAD	0x4e
#define DS18B20_CMD_READ_SCRATCHPAD	0xbe
#define DS18B20_CMD_COPY_SCRATCHPAD	0x48
#define DS18B20_CMD_RECAL_E2		0xb8
#define DS18B20_CMD_READ_PS		0xb4

Ds18b20::Ds18b20(Dongle *dongle, Dongle::Addr addr, float cal0, float cal100) :
	dongle(dongle), addr(addr) {
	offset = -cal0;
	scale = 100.0 / (cal100 - cal0);
}

Ds18b20::Ds18b20(Dongle *dongle, Dongle::Addr addr) :
	dongle(dongle), addr(addr), offset(0.0), scale(1.0) {
}

void Ds18b20::startConversion(void)
{
	dongle->reset();
	dongle->matchRom(addr);
	dongle->writeByte(DS18B20_CMD_CONVERT_T);
}

void Ds18b20::startAllConversion(void)
{
	dongle->reset();
	dongle->skipRom();
	dongle->writeByte(DS18B20_CMD_CONVERT_T);
}

bool Ds18b20::isConversionDone(void)
{
	return dongle->read() == 1;
}

float Ds18b20::getTemp(void)
{
	unsigned val;
	float temp;
	dongle->reset();
	dongle->matchRom(addr);
	dongle->writeByte(DS18B20_CMD_READ_SCRATCHPAD);


	val = dongle->readByte();
	val |= dongle->readByte() << 8;
	temp = (val >> 4) + (val & 0xf) * (1.0/16.0);
	temp -= offset;
	temp *= scale;
	return temp;
}

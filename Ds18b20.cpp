// Copyright 2013 Erik Gilling <konkers@konkers.net>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the Licene.

#include "Ds18b20.hpp"

#define DS18B20_CMD_CONVERT_T		0x44
#define DS18B20_CMD_WRITE_SCRATCHPAD	0x4e
#define DS18B20_CMD_READ_SCRATCHPAD	0xbe
#define DS18B20_CMD_COPY_SCRATCHPAD	0x48
#define DS18B20_CMD_RECAL_E2		0xb8
#define DS18B20_CMD_READ_PS		0xb4

Ds18b20::Ds18b20(Dongle *dongle, Dongle::Addr addr, std::string name, float cal0, float cal100) :
	dongle(dongle), addr(addr), name(name) {
	offset = -cal0;
	scale = 100.0 / (cal100 - cal0);
}

Ds18b20::Ds18b20(Dongle *dongle, Dongle::Addr addr) :
	dongle(dongle), addr(addr), offset(0.0), scale(1.0) {
	name = addr.getName();
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

void Ds18b20::updateTemp(void)
{
	unsigned val;
	float newTemp;

	dongle->reset();
	dongle->matchRom(addr);
	dongle->writeByte(DS18B20_CMD_READ_SCRATCHPAD);


	val = dongle->readByte();
	val |= dongle->readByte() << 8;
	newTemp = (val >> 4) + (val & 0xf) * (1.0/16.0);
	newTemp -= offset;
	newTemp *= scale;

	temp = newTemp;
}

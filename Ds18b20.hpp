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

#ifndef __DS18B20_HPP__
#define __DS18B20_HPP__

#include "Dongle.hpp"
#include "TempSensor.hpp"

class Ds18b20 : public TempSensor {
private:
	Dongle *dongle;
	Dongle::Addr addr;
	std::string name;
	float offset;
	float scale;

	float temp;
public:
	Ds18b20(Dongle *dongle, Dongle::Addr addr, std::string name, float cal0, float cal100);
	Ds18b20(Dongle *dongle, Dongle::Addr addr);

	void startConversion(void);
	void startAllConversion(void);

	bool isConversionDone(void);

	void updateTemp(void);

	virtual float getTemp(void) {
		return temp;
	}

	virtual const std::string getName(void) {
		return name;
	}

	Dongle::Addr getAddr(void) {
		return addr;
	}
};

#endif /* __DS18B20_HPP__ */

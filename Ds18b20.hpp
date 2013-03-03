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

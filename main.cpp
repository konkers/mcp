#include <stdio.h>

#include <vector>
using namespace std;

#include "Dongle.hpp"
#include "Ds18b20.hpp"

// 28 c5 c5 f4 03 00 00 01
//  0.0 = 0.250000
//  100.0 = 100

// 28 77 02 8d 02 00 00 8b
//  0.0 = 0.375000
//  100.0 = 99.562500

// 28 55 33 8d 02 00 00 1a
//  0.0 = 0.250000
//  100.0 = 99.750000

Ds18b20 *newSensor(Dongle *dongle, Dongle::Addr addr)
{
	if (addr == Dongle::Addr(0x28, 0xc5, 0xc5, 0xf4, 0x03, 0x00, 0x00, 0x01))
		return new Ds18b20(dongle, addr, 0.25, 100.0);
	else if (addr ==  Dongle::Addr(0x28, 0x77, 0x02, 0x8d, 0x02, 0x00, 0x00, 0x8b))
		return new Ds18b20(dongle, addr, 0.375, 99.5625);
	else if (addr ==  Dongle::Addr(0x28, 0x55, 0x33, 0x8d, 0x02, 0x00, 0x00, 0x1a))
		return new Ds18b20(dongle, addr, 0.25, 99.75);
	else if (addr.addr[0] == 0x28)
		return new Ds18b20(dongle, addr);
	else
		return NULL;
}


int main(int argc, char *argv[])
{
	Dongle d;
	int ret;
	int i;

	if (!d.connect())
		return 1;

	ret = d.enumerate();
	if (ret < 0)
		return 1;

	vector<Ds18b20 *> sensors;


	for (i = 0; i < ret; i++) {
		Dongle::Addr a = d.getAddr(i);
		Ds18b20 *ds = newSensor(&d, a);
		if (ds)
			sensors.push_back(ds);
	}

	while(1) {
		sensors[0]->startAllConversion();
		while (!sensors[0]->isConversionDone()) { }

		for (vector<Ds18b20 *>::iterator ds = sensors.begin();
		     ds != sensors.end();
		     ds++) {
			float temp = (*ds)->getTemp();
			Dongle::Addr a = (*ds)->getAddr();
			printf("%02x %02x %02x %02x %02x %02x %02x %02x = %f\n",
			       a.addr[0], a.addr[1], a.addr[2], a.addr[3],
			       a.addr[4], a.addr[5], a.addr[6], a.addr[7],
			       temp);
		}
	}

	return 0;
}

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <vector>
#include <map>
using namespace std;

#include "Dongle.hpp"
#include "Ds18b20.hpp"
#include "Pid.hpp"

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

bool running = true;

void signal_handler(int signum)
{
	running = false;
	signal(signum, SIG_DFL);
}

int main(int argc, char *argv[])
{
	Dongle d;
	int ret;
	int i;
	Pid pid(79.5, 50.0, 0, 0);

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	if (!d.connect())
		return 1;

	ret = d.enumerate();
	if (ret < 0)
		return 1;

	vector<Ds18b20 *> sensors;
	map<Dongle::Addr, Ds18b20 *> sensorMap;

	for (i = 0; i < ret; i++) {
		Dongle::Addr a = d.getAddr(i);
		Ds18b20 *ds = newSensor(&d, a);
		if (ds) {
			sensors.push_back(ds);
			sensorMap[a] = ds;
		}
	}

	Ds18b20 *heaterTemp = NULL;
	map<Dongle::Addr, Ds18b20 *>::const_iterator si =
		sensorMap.find(Dongle::Addr(0x28, 0xc5, 0xc5, 0xf4, 0x03, 0x00, 0x00, 0x01));
	if (si != sensorMap.end())
		heaterTemp = si->second;
	else
		printf("Could not find heater temp probe\n");

	while(running) {
		sensors[0]->startAllConversion();
		while (!sensors[0]->isConversionDone()) { }

#if 1
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
#endif
		if (heaterTemp) {
			float temp = heaterTemp->getTemp();
			float power = pid.update(temp);
			uint8_t power_1 = power * 255;
			d.setPower(power_1);
			printf("[%.2f] %.2f,%02x  %.2f,%.2f\n",
			       temp, power, power_1,  pid.getPkt_1(), pid.getEkt_1());
		}
		sleep(10);
	}

	printf("exiting\n");

	return 0;
}

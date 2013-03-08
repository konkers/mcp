#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <vector>
#include <map>
using namespace std;

#include "DongleThread.hpp"
#include "EventQueue.hpp"
#include "Pid.hpp"
#include "TimerThread.hpp"
#include "UsbDongle.hpp"
#include "WebServer.hpp"

Ds18b20 *newSensor(Dongle *dongle, Dongle::Addr addr)
{
	if (addr == Dongle::Addr(0x28, 0xc5, 0xc5, 0xf4, 0x03, 0x00, 0x00, 0x01))
		return new Ds18b20(dongle, addr, "RIMS", 0.25, 100.0);
	else if (addr ==  Dongle::Addr(0x28, 0x77, 0x02, 0x8d, 0x02, 0x00, 0x00, 0x8b))
		return new Ds18b20(dongle, addr, addr.getName(), 0.375, 99.5625);
	else if (addr ==  Dongle::Addr(0x28, 0x55, 0x33, 0x8d, 0x02, 0x00, 0x00, 0x1a))
		return new Ds18b20(dongle, addr, addr.getName(), 0.25, 99.75);
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
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	UsbDongle d;
	int ret, i;

	printf("1\n");
	if (!d.connect())
		return -1;

	printf("2\n");
	ret = d.enumerate();
	if (ret < 0)
		return -1;

	printf("3\n");
	Ds18b20 *heaterTemp = NULL;
	for (i = 0; i < ret; i++) {
		Dongle::Addr a = d.getAddr(i);
		if (a == Dongle::Addr(0x28, 0xc5, 0xc5, 0xf4, 0x03, 0x00, 0x00, 0x01))
			heaterTemp = newSensor(&d, a);
		printf("%i: %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
		       a.addr[0], a.addr[1], a.addr[2], a.addr[3],
		       a.addr[4], a.addr[5], a.addr[6], a.addr[7]);
	}


	Dongle::Addr outAddr = Dongle::Addr(0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	uint8_t out = 0x1;

	while (running) {
		d.reset();
		d.matchRom(outAddr);
		d.writeByte(0x4e);
		d.writeByte(out);
		out = ~out;
		i++;
		if (heaterTemp) {
			heaterTemp->startAllConversion();
			while (!heaterTemp->isConversionDone()) { }
			printf("heaterTemp: %f\n", heaterTemp->getTemp());
		}
		sleep(20);

	}

	return 0;
}

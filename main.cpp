#include <stdio.h>

#include "Dongle.hpp"
#include "Ds18b20.hpp"

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

	while(1) {
		for (i = 0; i < ret; i++) {
			Dongle::Addr a = d.getAddr(i);
			Ds18b20 ds(&d, a);

			ds.startConversion();
			while (!ds.isConversionDone()) { }
			float temp = ds.getTemp();

			printf("%d: %02x %02x %02x %02x %02x %02x %02x %02x = %f\n", i,
			       a.addr[0], a.addr[1], a.addr[2], a.addr[3],
			       a.addr[4], a.addr[5], a.addr[6], a.addr[7],
			       temp);
		}
	}

	return 0;
}

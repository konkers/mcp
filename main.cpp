#include <stdio.h>

#include "Dongle.hpp"

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

	for (i = 0; i < ret; i++) {
		Dongle::Addr a = d.getAddr(i);
		printf("%d: %02x %02x %02x %02x %02x %02x %02x %02x\n", i,
		       a.addr[0], a.addr[1], a.addr[2], a.addr[3],
		       a.addr[4], a.addr[5], a.addr[6], a.addr[7]);
	}

	return 0;
}

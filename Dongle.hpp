#ifndef __DONGLE_HPP__
#define __DONGLE_HPP__

#include <string.h>

#include <string>
#include <vector>

#include <libusb-1.0/libusb.h>

class Dongle {
private:
public:
	class Addr {
	public:
		uint8_t	addr[8];

		Addr() {
			memset(addr, 0, sizeof(addr));
		}

		Addr(const uint8_t *addr) {
			memcpy(this->addr, addr, sizeof(this->addr));
		}

		Addr(uint8_t a0, uint8_t a1, uint8_t a2, uint8_t a3,
		     uint8_t a4, uint8_t a5, uint8_t a6, uint8_t a7) {
			addr[0] = a0;
			addr[1] = a1;
			addr[2] = a2;
			addr[3] = a3;
			addr[4] = a4;
			addr[5] = a5;
			addr[6] = a6;
			addr[7] = a7;
		}

		Addr(const Addr &a) {
			memcpy(addr, a.addr, sizeof(addr));
		}

		friend bool operator==(const Addr &a, const Addr &b) {
			return !memcmp(a.addr, b.addr, 8);
		}
		friend bool operator<(const Addr &a, const Addr &b) {
			return memcmp(a.addr, b.addr, 8) < 0;
		}

		std::string getName(void) {
			char buf[8 * 3];
			snprintf(buf, sizeof(buf),
				 "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
				 addr[0], addr[1], addr[2], addr[3],
				 addr[4], addr[5], addr[6], addr[7]);
			return std::string(buf);
		}
	};

	virtual ~Dongle(){};

	virtual bool connect(void) = 0;

	virtual int enumerate(void) = 0;
	virtual int reset(void) = 0;
	virtual int matchRom(const Addr addr) = 0;
	virtual int skipRom(void) = 0;
	virtual int read(void) = 0;
	virtual int readByte(void) = 0;
	virtual int writeByte(uint8_t data) = 0;

	/* HACK until OW pwm device is made */
	virtual int setPower(uint8_t power) = 0;

	virtual Addr getAddr(unsigned n) = 0;
};

#endif /* __DONGLE_HPP__ */

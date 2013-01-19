#ifndef __DONGLE_HPP__
#define __DONGLE_HPP__

#include <vector>

#include <libusb-1.0/libusb.h>

class Dongle {
private:
	int			debug_level;
	libusb_context		*ctx;
	libusb_device_handle	*dev_handle;
	int			in_ep;
	int			out_ep;

	enum {
		OW_ENUMERATE	= 0x0,
		OW_RESET	= 0x1,
		OW_MATCH_ROM	= 0x2,
		OW_SKIP_ROM	= 0x3,
		OW_READ		= 0x4,
		OW_READ_BYTE	= 0x5,
		OW_WRITE_BYTE	= 0x6,
	};

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

		friend int operator==(const Addr &a, const Addr &b) {
			return !memcmp(a.addr, b.addr, 8);
		}

	};

private:
	std::vector<Addr> addrs;

	int debug(int level, const char *fmt, ...);
	bool openDevice(uint16_t vendor, uint16_t product);
	int doCommand(uint8_t *cmd, int cmd_len,
			      uint8_t *read_data, int read_data_len);
	int doCommand(uint8_t cmd, uint8_t *read_data, int read_data_len);

public:
	Dongle();
	~Dongle();

	bool connect(void);

	int enumerate(void);
	int reset(void);
	int matchRom(const Addr addr);
	int skipRom(void);
	int read(void);
	int readByte(void);
	int writeByte(uint8_t data);

	Addr getAddr(unsigned n);
};

#endif /* __DONGLE_HPP__ */

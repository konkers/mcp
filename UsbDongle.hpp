#ifndef __USBDONGLE_HPP__
#define __USBDONGLE_HPP__

#include <string.h>

#include <string>
#include <vector>

#include <libusb-1.0/libusb.h>

#include "Dongle.hpp"

class UsbDongle : public Dongle {
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

		/* HACK until OW pwm device is made */
		SET_POWER	= 0x80,
	};

	std::vector<Addr> addrs;

	int debug(int level, const char *fmt, ...);
	bool openDevice(uint16_t vendor, uint16_t product);
	int doCommand(uint8_t *cmd, int cmd_len,
			      uint8_t *read_data, int read_data_len);
	int doCommand(uint8_t cmd, uint8_t *read_data, int read_data_len);

public:
	UsbDongle();
	virtual ~UsbDongle();

	virtual bool connect(void);

	virtual int enumerate(void);
	virtual int reset(void);
	virtual int matchRom(const Addr addr);
	virtual int skipRom(void);
	virtual int read(void);
	virtual int readByte(void);
	virtual int writeByte(uint8_t data);

	/* HACK until OW pwm device is made */
	virtual int setPower(uint8_t power);

	virtual Addr getAddr(unsigned n);
};

#endif /* __USBDONGLE_HPP__ */

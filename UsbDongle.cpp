//
// Copyright (C) 2013 Erik Gilling
// Based on code Copyright (C) 2012 The Android Open Source Project
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//  * Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in
//    the documentation and/or other materials provided with the
//    distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
// OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
// AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// SUCH DAMAGE.
//
#include <stdarg.h>
#include <stdio.h>

#include "UsbDongle.hpp"

UsbDongle::UsbDongle() :
	debug_level(1), ctx(NULL), dev_handle(NULL), in_ep(-1), out_ep(-1)
{

}

UsbDongle::~UsbDongle()
{
	if (dev_handle)
		libusb_close(dev_handle);
}

int UsbDongle::debug(int level, const char *fmt, ...)
{
	int ret;

	if (level > debug_level)
		return 0;

	va_list va;
	va_start(va, fmt);
	printf("[UsbDongle] ");
	ret = vprintf(fmt, va);
	va_end(va);

	return ret;
}

bool UsbDongle::openDevice(uint16_t vendor, uint16_t product)
{
	libusb_device **dev_list;
	ssize_t len;
	int i;
	int err;

	len = libusb_get_device_list(ctx, &dev_list);
	if (len < 0) {
		debug(0, "Failed to get device list: %s\n", libusb_error_name(len));
		return NULL;
	}
	for (i = 0; i < len; i++) {
		struct libusb_device_descriptor dev_desc;

		err = libusb_get_device_descriptor(dev_list[i], &dev_desc);
		if (err < 0)
			continue;

		if (dev_desc.idVendor == vendor && dev_desc.idProduct == product) {
			libusb_device *dev = libusb_ref_device(dev_list[i]);

			err = libusb_open(dev, &dev_handle);
			if (err < 0) {
				debug(0, "Failed to open device: %s\n", libusb_error_name(err));
				goto err0;
			}

			err = libusb_claim_interface(dev_handle, 0);
			if (err < 0) {
				debug(0, "Failed to claim interface: %s\n", libusb_error_name(err));
				goto err1;
			}

			struct libusb_config_descriptor *cfg_desc;
			err = libusb_get_active_config_descriptor(dev, &cfg_desc);
			if (err < 0) {
				debug(0, "Failed to get config descriptor: %s\n",
				      libusb_error_name(err));
				goto err1;
			}

			const struct libusb_interface_descriptor *ifc_desc;
			ifc_desc = &cfg_desc->interface[0].altsetting[0];

			int n;
			for (n = 0; n < ifc_desc->bNumEndpoints; n++) {
				const struct libusb_endpoint_descriptor *ep =
					&ifc_desc->endpoint[n];

				if ((ep->bmAttributes & 0x3) ==
				    LIBUSB_TRANSFER_TYPE_BULK) {
					if (in_ep < 0 && (ep->bEndpointAddress & 0x80) == LIBUSB_ENDPOINT_IN)
						in_ep = ep->bEndpointAddress;
					if (out_ep < 0 && (ep->bEndpointAddress & 0x80) == LIBUSB_ENDPOINT_OUT)
						out_ep = ep->bEndpointAddress;
				}
			}
			libusb_free_config_descriptor(cfg_desc);
			break;

		}
	}

	if (i == len) {
		debug(0, "Did not find device matching %04x:%04x\n", vendor, product);
		goto err1;
	}

	if (in_ep < 0 || out_ep < 0) {
		debug(0, "Did not find bulk endpoints\n");
		goto err1;
	}
	libusb_free_device_list(dev_list, 0);
	return true;

err1:
	if (dev_handle)
		libusb_close(dev_handle);

err0:
	libusb_free_device_list(dev_list, 0);
	return false;
}

int UsbDongle::doCommand(uint8_t *cmd, int cmd_len,
		      uint8_t *read_data, int read_data_len)
{
	int len = 0;
	int err;

	err = libusb_bulk_transfer(dev_handle, out_ep,
				   cmd, cmd_len, &len,
				   1000000);
	if (err < 0)
		return err;

	if (len != cmd_len)
		return 0;

	if (read_data_len > 0) {
		err = libusb_bulk_transfer(dev_handle, in_ep,
					   read_data, read_data_len,
					   &len, 1000000);
		if (err < 0) {
			debug(0, "Bulk transfer error: %s\n", libusb_error_name(err));
			return err;
		}
	}

	return len;
}

int UsbDongle::doCommand(uint8_t cmd, uint8_t *read_data, int read_data_len)
{
	return doCommand(&cmd, 1, read_data, read_data_len);
}


int UsbDongle::enumerate(void)
{
	uint8_t addrs_buf[4096];
	int len;
	int i;

	len = doCommand(OW_ENUMERATE, addrs_buf, sizeof(addrs_buf));
	if (len < 0)
		return len;

	if ((len % 8) != 0) {
		debug(1, "enumerate returned %d bytes.  truncated to %d\n",
		      len, len - (len % 8));
		len -= len % 8;
	}

	addrs.clear();
	for (i = 0; i < (len / 8) ; i++) {
		addrs.push_back(Addr(addrs_buf + i * 8));
	}
	return i;
}

int UsbDongle::reset(void)
{
	uint8_t state;
	int err;

	err = doCommand(OW_RESET, &state, 1);
	if (err < 0)
		return err;

	return state;
}

int UsbDongle::matchRom(const Addr addr)
{
	uint8_t cmd[9];

	cmd[0] = OW_MATCH_ROM;
	memcpy(&cmd[1], addr.addr, 8);

	return doCommand(cmd, sizeof(cmd), NULL, 0);
}

int UsbDongle::skipRom(void)
{
	return doCommand(OW_SKIP_ROM, NULL, 0);
}

int UsbDongle::read(void)
{
	uint8_t state;
	int err;

	err = doCommand(OW_READ, &state, 1);
	if (err < 0)
		return err;

	return state;
}

int UsbDongle::readByte(void)
{
	uint8_t byte;
	int err;

	err = doCommand(OW_READ_BYTE, &byte, 1);
	if (err < 0)
		return err;

	return byte;
}

int UsbDongle::writeByte(uint8_t data)
{
	uint8_t cmd[2];

	cmd[0] = OW_WRITE_BYTE;
	cmd[1] = data;

	return doCommand(cmd, sizeof(cmd), NULL, 0);
}

/* HACK until OW pwm device is made */
int UsbDongle::setPower(uint8_t power)
{
	uint8_t cmd[2];

	cmd[0] = SET_POWER;
	cmd[1] = power;

	return doCommand(cmd, sizeof(cmd), NULL, 0);
}

bool UsbDongle::connect(void)
{
	int err;

	err = libusb_init(&ctx);
	if (err < 0) {
		debug(0, "USB init failed: %s\n", libusb_error_name(err));
		return false;
	}
	if (!openDevice(0x18d1, 0xbeef))
		return false;

	return true;
}

UsbDongle::Addr UsbDongle::getAddr(unsigned n)
{
	if (n < addrs.size())
		return addrs[n];
	return Addr();
}

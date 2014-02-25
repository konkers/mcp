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

#include <stdarg.h>
#include <stdio.h>

#include "UsbDongle.hpp"

UsbDongle::UsbDongle()
{
    m_buses.push_back(new Bus(*this, 0));
    m_buses.push_back(new Bus(*this, 1));
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
    // printf("[UsbDongle] ");
    ret = vprintf(fmt, va);
    va_end(va);

    return ret;
}

bool UsbDongle::connect(void)
{
    int err;

    if (m_connected) {
        return true;
    }

    err = libusb_init(&ctx);
    if (err < 0) {
        debug(0, "USB init failed: %s\n", libusb_error_name(err));
        return false;
    }
    if (!openDevice(0x18d1, 0xbeef))
        return false;

    m_connected = true;
    return true;
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
        return false;
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

    int i;
    for (i = 0; i < cmd_len; i++) {
        debug(3, "%x ", cmd[i]);
    }
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

        debug(3, "-> ");

        for (i = 0; i < len; i++) {
            debug(3, "%x ", read_data[i]);
        }
    }

    debug(3, "\n");
    return len;
}

int UsbDongle::doCommand(uint8_t cmd, uint8_t index, uint8_t *read_data, int read_data_len)
{
    uint8_t data[2];
    data[0] = cmd;
    data[1] = index;
    return doCommand(data, 2, read_data, read_data_len);
}


// ----------------------------------------------------------------------------
// UsbDongle::Bus
// ----------------------------------------------------------------------------
UsbDongle::Bus::Bus(UsbDongle &parent, size_t index)
    : m_parent(parent),
      m_index(index) {
}

UsbDongle::Bus::~Bus() {
}

int UsbDongle::Bus::enumerate(void)
{
    uint8_t addrs_buf[4096];
    int len;
    int i;

    len = m_parent.doCommand(OW_ENUMERATE, m_index, addrs_buf, sizeof(addrs_buf));
    if (len < 0)
        return len;

    if ((len % 8) != 0) {
        m_parent.debug(1, "enumerate returned %d bytes.  truncated to %d\n",
              len, len - (len % 8));
        len -= len % 8;
    }

    m_addrs.clear();
    for (i = 0; i < (len / 8) ; i++) {
        m_addrs.push_back(Addr(addrs_buf + i * 8));
    }
    return i;
}

int UsbDongle::Bus::reset(void)
{
    uint8_t state;
    int err;

    err = m_parent.doCommand(OW_RESET, m_index, &state, 1);
    if (err < 0)
        return err;

    return state;
}

int UsbDongle::Bus::matchRom(const Addr addr)
{
    uint8_t cmd[10];

    cmd[0] = OW_MATCH_ROM;
    cmd[1] = m_index;
    memcpy(&cmd[2], addr.addr, 8);

    return m_parent.doCommand(cmd, sizeof(cmd), NULL, 0);
}

int UsbDongle::Bus::skipRom(void)
{
    return m_parent.doCommand(OW_SKIP_ROM, m_index, NULL, 0);
}

int UsbDongle::Bus::read(void)
{
    uint8_t state;
    int err;

    err = m_parent.doCommand(OW_READ, m_index, &state, 1);
    if (err < 0)
        return err;

    return state;
}

int UsbDongle::Bus::readByte(void)
{
    uint8_t byte;
    int err;

    err = m_parent.doCommand(OW_READ_BYTE, m_index, &byte, 1);
    if (err < 0)
        return err;

    return byte;
}

int UsbDongle::Bus::writeByte(uint8_t data)
{
    uint8_t cmd[3];

    cmd[0] = OW_WRITE_BYTE;
    cmd[1] = m_index;
    cmd[2] = data;

    return m_parent.doCommand(cmd, sizeof(cmd), NULL, 0);
}

/* HACK until OW pwm device is made */
int UsbDongle::Bus::setPower(uint8_t power)
{
    uint8_t cmd[3];

    cmd[0] = SET_POWER;
    cmd[1] = m_index;
    cmd[2] = power;

    return m_parent.doCommand(cmd, sizeof(cmd), NULL, 0);
}

bool UsbDongle::Bus::connect(void)
{
    return m_parent.connect();
}

Dongle::Addr UsbDongle::Bus::getAddr(unsigned n)
{
    if (n < m_addrs.size())
        return m_addrs[n];
    return Addr();
}

// Copyright 2013 Erik Gilling <konkers@konkers.net>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the Licene.

#ifndef __USBDONGLE_HPP__
#define __USBDONGLE_HPP__

#include <string.h>

#include <string>
#include <vector>

#include <libusb-1.0/libusb.h>

#include "Dongle.hpp"

class UsbDongle {
public:
    UsbDongle();
    ~UsbDongle();

    Dongle *getBus(size_t i) {
        if (i >= m_buses.size()) {
            return nullptr;
        }

        return m_buses[i];
    }

private:
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

    class Bus : public Dongle {
    public:
        Bus(UsbDongle &parent, size_t index);
        virtual ~Bus();

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

    private:
        UsbDongle           &m_parent;
        size_t              m_index;
        std::vector<Addr>   m_addrs;

    };

    int debug(int level, const char *fmt, ...);
    bool connect();
    bool openDevice(uint16_t vendor, uint16_t product);
    int doCommand(uint8_t *cmd, int cmd_len,
                  uint8_t *read_data, int read_data_len);
    int doCommand(uint8_t cmd, uint8_t index, uint8_t *read_data, int read_data_len);

    int			debug_level = 1;
    libusb_context		*ctx = nullptr;
    libusb_device_handle	*dev_handle = nullptr;
    int			in_ep = -1;
    int			out_ep = -1;
    bool                m_connected = false;

    std::vector<Bus *>  m_buses;

};

#endif /* __USBDONGLE_HPP__ */

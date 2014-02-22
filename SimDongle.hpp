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

#ifndef __SIMDONGLE_HPP__
#define __SIMDONGLE_HPP__

#include <string>
#include <vector>

#include <chrono>

#include "Dongle.hpp"

class SimDongle : public Dongle {
private:
    std::vector<Addr> addrs;
    enum State {
        RESET,
        SKIP_ROM,
        MATCH_ROM,
        READ_SCRATCHPAD,
        CONVERTING,
    };
    State state;
    Addr matchAddr;
    int scratchPadAddr;

    std::chrono::system_clock::time_point conversionStart;

    float simTemp;
    float simTempDelta;
    float simTempMin;
    float simTempMax;

    void startConversion(void) {
        conversionStart = std::chrono::system_clock::now();
        state = CONVERTING;
        simTemp += simTempDelta;
        if (simTemp < simTempMin || simTemp > simTempMax)
            simTempDelta = -simTempDelta;
    }

    uint16_t tempTo18b20(float temp) {
        uint16_t val = 0x0;
        // brain dead way of handling sign extension
        if (temp < 0) {
            temp = -temp;
            val = 0xf800;
        }

        val |= (uint16_t)(temp * 16) & 0x7ff;

        return val;
    }

public:
    SimDongle();
    virtual ~SimDongle();

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


#endif /* __SIMDONGLE_HPP__ */

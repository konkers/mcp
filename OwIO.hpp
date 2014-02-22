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

#ifndef __OWIO_HPP__
#define __OWIO_HPP__

#include <string>

#include "Dongle.hpp"
#include "State.hpp"

class OwIO {
private:
    class OwIOOutput : public State::Output {
    private:
        OwIO *board;
        unsigned index;
    public:
        OwIOOutput(OwIO *board, unsigned index, std::string name);

        virtual void setValue(unsigned value);
    };

    Dongle *dongle;

    Dongle::Addr addr;
    std::string name;

    unsigned numOutputs;
    OwIOOutput **outputs;

    uint8_t outputState;

    void setOutput(unsigned index, unsigned value);
public:
    OwIO(Dongle *dongle, Dongle::Addr addr, std::string name, unsigned numOutputs, const char *names[]);
    ~OwIO();

    void sync(void);

    const std::string getName(void) {
        return name;
    }

    Dongle::Addr getAddr(void) {
        return addr;
    }
};

#endif /* __OWIO_HPP__ */

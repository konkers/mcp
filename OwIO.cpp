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

#include <sys/time.h>

#include "OwIO.hpp"

using namespace std;

#define OWCMD_WRITE_SCRATCHPAD	0x4e
#define DS18B20_CMD_READ_SCRATCHPAD	0xbe

OwIO::OwIOOutput::OwIOOutput(OwIO *board, unsigned index, std::string name)
: State::Output(name, 0xff), board(board), index(index)
{

}

void OwIO::OwIOOutput::setValue(unsigned value)
{
    State::Output::setValue(value);
}

OwIO::OwIO(Dongle *dongle, Dongle::Addr addr, std::string name, unsigned numOutputs, const char *names[]) :
    dongle(dongle), addr(addr), name(name), numOutputs(numOutputs)
{
    int i;
    State::State *state = State::getState();

    outputs = new OwIOOutput *[numOutputs];

    for (i = 0; i < numOutputs; i++) {
        outputs[i] = new OwIOOutput(this, i, names[i]);
        state->addOutput(outputs[i]);
    }
}

OwIO::~OwIO()
{
    int i;
    State::State *state = State::getState();

    for (i = 0; i < numOutputs; i++) {
        state->removeOutput(outputs[i]->getName());
        delete outputs[i];
    }
    delete outputs;
}

void OwIO::sync(void)
{
    uint8_t output_state = 0;
    struct timeval now;
    gettimeofday(&now, NULL);
    uint64_t usecs = now.tv_usecs;
    uint8_t current_val = usecs * 255 / 1000000;

    for (i = 0; i < numOutputs; i++) {
        unsigned value = outputs[i]->getValue();
        unsigned index = outputs[i]->getIndex();

        if (value >= current_val) {
            output_state |= 1 << index;
        }
    }

    dongle->reset();
    dongle->matchRom(addr);
    dongle->writeByte(OWCMD_WRITE_SCRATCHPAD);
    dongle->writeByte(output_state);
}

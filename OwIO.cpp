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

#include "OwIO.hpp"

using namespace std;

#define OWCMD_WRITE_SCRATCHPAD	0x4e
#define DS18B20_CMD_READ_SCRATCHPAD	0xbe

OwIO::OwIOOutput::OwIOOutput(OwIO *board, unsigned index, std::string name)
	: State::Output(name, 2), board(board), index(index)
{

}

void OwIO::OwIOOutput::setValue(unsigned value)
{
	State::Output::setValue(value);
	board->setOutput(index, value);
}

OwIO::OwIO(Dongle *dongle, Dongle::Addr addr, std::string name, unsigned numOutputs, const char *names[]) :
	dongle(dongle), addr(addr), name(name), numOutputs(numOutputs), outputState(0)
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

void OwIO::setOutput(unsigned index, unsigned value)
{
	if (value)
		outputState |= 1 << index;
	else
		outputState &= ~(1 << index);
}

void OwIO::sync(void)
{
	dongle->reset();
	dongle->matchRom(addr);
	dongle->writeByte(OWCMD_WRITE_SCRATCHPAD);
	dongle->writeByte(outputState);
}

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

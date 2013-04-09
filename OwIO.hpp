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

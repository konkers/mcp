#ifndef __DONGLETHREAD__HPP__
#define __DONGLETHREAD__HPP__

#include <vector>
#include <map>

#include "Dongle.hpp"
#include "Ds18b20.hpp"
#include "EventQueue.hpp"
#include "OwIO.hpp"
#include "State.hpp"
#include "Thread.hpp"

class DongleThread : public Thread {
private:
	EventQueue *eventQueue;

	std::vector<Ds18b20 *> sensors;
	std::vector<OwIO *> outputs;

	Dongle *d;
	Thread::Mutex dongleLock;

	Ds18b20 *newSensor(Dongle *dongle, Dongle::Addr addr);
	OwIO *newOwIO(Dongle *dongle, Dongle::Addr addr);

	bool doConversion;
	Thread::Condition convCond;
protected:
	virtual int run(void);
	virtual void signalStop(void);

public:
	DongleThread(Dongle *dongle, EventQueue *queue);
	virtual ~DongleThread();

	void startConversion(void);
	void setPower(uint8_t power);

	void writeByte(Dongle::Addr addr, uint8_t cmd, uint8_t data);

	void sync(void);
};

#endif /* __DONGLETHREAD__HPP__ */

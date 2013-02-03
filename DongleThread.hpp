#ifndef __DONGLETHREAD__HPP__
#define __DONGLETHREAD__HPP__

#include <vector>
#include <map>

#include "Dongle.hpp"
#include "Ds18b20.hpp"
#include "EventQueue.hpp"
#include "Thread.hpp"

class DongleThread : public Thread {
private:
	EventQueue *eventQueue;

	std::vector<Ds18b20 *> sensors;
	std::map<Dongle::Addr, Ds18b20 *> sensorMap;
	Ds18b20 *heaterTemp;

	Dongle d;
	Thread::Mutex dongleLock;

	Ds18b20 *newSensor(Dongle *dongle, Dongle::Addr addr);

	bool doConversion;
	Thread::Condition convCond;
protected:
	virtual int run(void);
	virtual void signalStop(void);

public:
	DongleThread(EventQueue *queue);
	virtual ~DongleThread();

	void startConversion(void);
	void setPower(uint8_t power);

};

#endif /* __DONGLETHREAD__HPP__ */

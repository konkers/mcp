
#include "DongleThread.hpp"

using namespace std;

DongleThread::DongleThread(EventQueue *queue) :
	Thread(), eventQueue(queue), doConversion(false)
{
}


DongleThread::~DongleThread()
{
	//XXX need to clear sensor list
}


// 28 c5 c5 f4 03 00 00 01
//  0.0 = 0.250000
//  100.0 = 100

// 28 77 02 8d 02 00 00 8b
//  0.0 = 0.375000
//  100.0 = 99.562500

// 28 55 33 8d 02 00 00 1a
//  0.0 = 0.250000
//  100.0 = 99.750000

Ds18b20 *DongleThread::newSensor(Dongle *dongle, Dongle::Addr addr)
{
	if (addr == Dongle::Addr(0x28, 0xc5, 0xc5, 0xf4, 0x03, 0x00, 0x00, 0x01))
		return new Ds18b20(dongle, addr, "RIMS", 0.25, 100.0);
	else if (addr ==  Dongle::Addr(0x28, 0x77, 0x02, 0x8d, 0x02, 0x00, 0x00, 0x8b))
		return new Ds18b20(dongle, addr, addr.getName(), 0.375, 99.5625);
	else if (addr ==  Dongle::Addr(0x28, 0x55, 0x33, 0x8d, 0x02, 0x00, 0x00, 0x1a))
		return new Ds18b20(dongle, addr, addr.getName(), 0.25, 99.75);
	else if (addr.addr[0] == 0x28)
		return new Ds18b20(dongle, addr);
	else
		return NULL;
}

int DongleThread::run(void)
{
	int ret;
	int i;
	State *state = State::getState();

	printf("1\n");
	if (!d.connect())
		return -1;

	printf("2\n");
	ret = d.enumerate();
	if (ret < 0)
		return -1;

	printf("3\n");
	for (i = 0; i < ret; i++) {
		Dongle::Addr a = d.getAddr(i);
		Ds18b20 *ds = newSensor(&d, a);
		if (ds) {
			sensors.push_back(ds);
			state->addTempSensor(ds->getName());
		}
	}
	printf("4\n");

#if 0
	auto si = sensorMap.find(Dongle::Addr(0x28, 0xc5, 0xc5, 0xf4, 0x03, 0x00, 0x00, 0x01));
	if (si != sensorMap.end()) {
		heaterTemp = si->second;
	} else {
		printf("Could not find heater temp probe\n");
		return -1;
	}
#endif

	while (running) {
		convCond.lock();
		while (running && !doConversion)
			convCond.wait();
		doConversion = false;
		convCond.unlock();

		if (!running)
			break;

		dongleLock.lock();
		sensors[0]->startAllConversion();
		while (!sensors[0]->isConversionDone()) {
			if (!running) {
				dongleLock.unlock();
				break;
			}
		}

		for (auto sensor : sensors) {
			sensor->updateTemp();
			state->updateTempSensor(sensor->getName(), sensor->getTemp());
		}

		dongleLock.unlock();

		EventQueue::StateUpdateEvent *event =
			new EventQueue::StateUpdateEvent();
		eventQueue->postEvent(event);
	}

	return 0;
}

void DongleThread::signalStop(void)
{
	convCond.lock();
	convCond.signal();
	convCond.unlock();
}

void DongleThread::startConversion(void)
{
	convCond.lock();
	doConversion = true;
	convCond.signal();
	convCond.unlock();
}

void DongleThread::setPower(uint8_t power)
{
	dongleLock.lock();
	d.setPower(power);
	dongleLock.unlock();
	printf("power set to %02x\n", power);
}

void DongleThread::writeByte(Dongle::Addr addr, uint8_t cmd, uint8_t data)
{
	dongleLock.lock();
	d.matchRom(addr);
	d.writeByte(cmd);
	d.writeByte(data);
	dongleLock.unlock();
}

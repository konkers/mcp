#include <unistd.h>

#include "DongleThread.hpp"

using namespace std;

DongleThread::DongleThread(Dongle *dongle, EventQueue *queue) :
	Thread(), eventQueue(queue), d(dongle), doConversion(false)
{
}


DongleThread::~DongleThread()
{
	//XXX need to clear sensor and output list
	delete d;
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
		return new Ds18b20(dongle, addr, "boil", 0.25, 100.0);
	else if (addr ==  Dongle::Addr(0x28, 0x77, 0x02, 0x8d, 0x02, 0x00, 0x00, 0x8b))
		return new Ds18b20(dongle, addr, addr.getName(), 0.375, 99.5625);
	else if (addr ==  Dongle::Addr(0x28, 0x55, 0x33, 0x8d, 0x02, 0x00, 0x00, 0x1a))
		return new Ds18b20(dongle, addr, "mashtun", 0.25, 99.75);
	else if (addr ==  Dongle::Addr(0x28, 0x99, 0xbd, 0xf4, 0x03, 0x00, 0x00, 0x01))
		return new Ds18b20(dongle, addr, "RIMS", 0.25, 99.75);
	else if (addr.addr[0] == 0x28)
		return new Ds18b20(dongle, addr);
	else
		return NULL;
}

OwIO *DongleThread::newOwIO(Dongle *dongle, Dongle::Addr addr)
{
	if (addr == Dongle::Addr(0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)) {
		const char *names[] = {"", "bk", "mlt", "hlt"};
		return new OwIO(dongle, addr, "valve_board", 4, names);
	} else {
		return NULL;
	}
}

int DongleThread::run(void)
{
	int ret;
	int i;
	State *state = State::getState();

	printf("1\n");
	if (!d->connect())
		return -1;

	printf("2\n");
	ret = d->enumerate();
	if (ret < 0)
		return -1;

	printf("3\n");
	for (i = 0; i < ret; i++) {
		Dongle::Addr a = d->getAddr(i);
		Ds18b20 *ds = newSensor(d, a);
		if (ds) {
			sensors.push_back(ds);
			state->addTemp(ds->getName());
		}

		OwIO *ow = newOwIO(d, a);
		if (ow)
			outputs.push_back(ow);
	}
	printf("4\n");

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
			usleep(100000);
			if (!running) {
				dongleLock.unlock();
				break;
			}
		}

		for (auto sensor : sensors) {
			sensor->updateTemp();
			state->updateTemp(sensor->getName(), sensor->getTemp());
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
	d->setPower(power);
	dongleLock.unlock();
	printf("power set to %02x\n", power);
}

void DongleThread::writeByte(Dongle::Addr addr, uint8_t cmd, uint8_t data)
{
	dongleLock.lock();
	d->matchRom(addr);
	d->writeByte(cmd);
	d->writeByte(data);
	dongleLock.unlock();
}

void DongleThread::sync(void)
{
	dongleLock.lock();
	for (auto output : outputs)
		output->sync();
	dongleLock.unlock();
}

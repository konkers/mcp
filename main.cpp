#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <vector>
#include <map>
using namespace std;

#include "DongleThread.hpp"
#include "EventQueue.hpp"
#include "Pid.hpp"
#include "TimerThread.hpp"
#include "WebServer.hpp"

Pid pid(0, 50.0, 0, 0);
bool running = true;

void signal_handler(int signum)
{
	running = false;
	signal(signum, SIG_DFL);
}

int main(int argc, char *argv[])
{
	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	EventQueue queue;

	WebServer server(8080, &pid, &queue);
	DongleThread dongle(&queue);
	TimerThread timer(&queue);

	timer.start();
	dongle.start();

	while(running) {
		EventQueue::Event *event = queue.getEvent();
		switch (event->getType()) {
		case EventQueue::Event::Type::tick:
			dongle.startConversion();
			break;

		case EventQueue::Event::Type::pidUpdate:
		{
			EventQueue::PidUpdateEvent *pidEvent =
				static_cast<EventQueue::PidUpdateEvent *>(event);
			printf("pid update %f %f %f %f\n",
			       pidEvent->getSetPoint(),
			       pidEvent->getP(),
			       pidEvent->getI(),
			       pidEvent->getD());
			pid.setSetPoint(pidEvent->getSetPoint());
			pid.setP(pidEvent->getP());
			pid.setI(pidEvent->getI());
			pid.setD(pidEvent->getD());
			pid.reset();
			break;
		}

		case EventQueue::Event::Type::tempUpdate:
		{
			EventQueue::TempUpdateEvent *tempEvent =
				static_cast<EventQueue::TempUpdateEvent *>(event);
			printf("temp %f\n", tempEvent->getTemp());
			float power = pid.update(tempEvent->getTemp());
			uint8_t power_1 = power * 255;
			dongle.setPower(power_1);

			break;
		}

		default:
			printf("unknown event type\n");
			break;
		}

		delete(event);
	}

	printf("exiting\n");
	timer.stop();
	dongle.stop();
	return 0;
}

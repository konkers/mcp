
#include "TimerThread.hpp"


TimerThread::TimerThread(EventQueue *queue) :
	Thread(), eventQueue(queue) {

}

TimerThread::~TimerThread()
{

}

int TimerThread::run(void)
{
	while(running) {
		sleep(1); // XXX: convert to new C++ chrono api
		EventQueue::TickEvent *event = new EventQueue::TickEvent();
		eventQueue->postEvent(event);
	}

	return 0;
}

#ifndef __TIMERTHREAD__HPP__
#define __TIMERTHREAD__HPP__

#include "EventQueue.hpp"
#include "Thread.hpp"

class TimerThread : public Thread {
private:
	EventQueue *eventQueue;

protected:
	virtual int run(void);

public:
	TimerThread(EventQueue *queue);
	virtual ~TimerThread();
};

#endif /* __TIMERTHREAD__HPP__ */

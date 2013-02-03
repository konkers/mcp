
#include "EventQueue.hpp"
#include "Thread.hpp"

EventQueue::EventQueue()
{

}

EventQueue::~EventQueue()
{
	while (!events.empty()) {
		EventQueue::Event *event = events.front();
		events.pop_front();
		delete event;
	}
}

EventQueue::Event *EventQueue::getEvent(void)
{
	EventQueue::Event *event;

	eventsCond.lock();
	while (events.empty())
		eventsCond.wait();

	event = events.front();
	events.pop_front();
	eventsCond.unlock();

	return event;
}

void EventQueue::postEvent(EventQueue::Event *event)
{
	eventsCond.lock();

	events.push_back(event);

	eventsCond.signal();
	eventsCond.unlock();
}

#ifndef __EVENTQUEUE_HPP__
#define __EVENTQUEUE_HPP__

#include <deque>

#include "Thread.hpp"

class EventQueue {
public:
	class Event {
	public:
		enum class Type {none, tick, stateUpdate, pidUpdate};

	private:
		Type type;

	protected:
		Event(Type type) : type(type){
		}

	public:
		Type getType(void) {
			return type;
		}
	};

	class TickEvent : public Event {
	public:
		TickEvent() : Event(Type::tick) {
		}
	};

	class StateUpdateEvent : public Event {
	private:
	public:
		StateUpdateEvent() :
			Event(Type::stateUpdate){
		}
	};

	class PidUpdateEvent : public Event {
	private:
		float setPoint, p, i, d;
	public:
		PidUpdateEvent(float setPoint, float p, float i, float d) :
			Event(Type::pidUpdate), setPoint(setPoint), p(p), i(i), d(d) {
		}

		float getSetPoint(void) {
			return setPoint;
		}

		float getP(void) {
			return p;
		}

		float getI(void) {
			return i;
		}

		float getD(void) {
			return d;
		}
	};

private:
	std::deque<Event *> events;
	Thread::Condition eventsCond;

public:
	EventQueue();
	~EventQueue();

	// both of the below transfer ownership of Event
	Event *getEvent(void);
	void postEvent(Event *event);
};

#endif /* __EVENTQUEUE_HPP__ */

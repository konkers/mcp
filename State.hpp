#ifndef __STATE_HPP__
#define __STATE_HPP__

#include <deque>
#include <map>
#include <string>

#include "Thread.hpp"

class State {
public:
	class Temp {
	private:
		std::string	name;
		float		temp;

		Thread::RWLock	lock;

	public:
		Temp(std::string name) : name(name), temp(0.0) {};

		const std::string getName(void) {
			return name;
		}

		float getTemp(void) {
			return temp;
		}

		void setTemp(float t) {
			temp = t;
		}
	};

private:
	std::map<std::string, Temp *> tempSensorMap;

	Thread::RWLock		lock;

	static State state;
	State();
public:
	~State();

	static State *getState(void) {
		return &state;
	}

	void rdlock(void) {
		lock.rdlock();
	}

	void wrlock(void) {
		lock.wrlock();
	}

	void unlock(void) {
		lock.unlock();
	}

	void addTempSensor(std::string name);
	void updateTempSensor(std::string name, float temp);
	void removeTempSensor(std::string name);

	float getTemp(std::string name);

	// must hold rdlock while using
	std::map<std::string, Temp *> *getTempSensorMap(void) {
		return &tempSensorMap;
	}
};

#endif /* __STATE_HPP__ */

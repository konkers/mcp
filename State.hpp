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


	class Output {
	private:
		std::string name;
		unsigned maxValue;
		unsigned value;
	public:

		Output(std::string name, unsigned maxValue) :
		name(name), maxValue(maxValue), value(0) {
		}

		virtual std::string getName(void) {
			return name;
		}

		virtual unsigned getMaxValue(void) {
			return maxValue;
		}

		virtual unsigned getValue(void) {
			return value;
		}

		virtual void setValue(unsigned value) {
			this-> value = value;
		}
	};

private:
	std::map<std::string, Temp *> tempMap;
	std::map<std::string, Output *> outputMap;

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

	void addTemp(std::string name);
	void updateTemp(std::string name, float temp);
	void removeTemp(std::string name);
	float getTemp(std::string name);

	void addOutput(Output *output);
	void updateOutput(std::string name, unsigned value);
	void removeOutput(std::string name);
	unsigned getOutput(std::string);


	// must hold rdlock while using
	std::map<std::string, Temp *> *getTempMap(void) {
		return &tempMap;
	}

	// must hold rdlock while using
	std::map<std::string, Output *> *getOutputMap(void) {
		return &outputMap;
	}

};

#endif /* __STATE_HPP__ */

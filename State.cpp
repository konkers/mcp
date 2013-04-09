#include "State.hpp"

State State::state;

State::State()
{

}

State::~State()
{
	for (auto i: tempMap) {
		delete i.second;
	}
	for (auto i: outputMap) {
		delete i.second;
	}

}

void State::addTemp(std::string name)
{
	Temp *temp;

	wrlock();
	if (tempMap.find(name) == tempMap.end()) {
		temp = new Temp(name);
		tempMap.insert(std::make_pair(name, temp));
	}
	unlock();
}

void State::updateTemp(std::string name, float temp)
{
	rdlock();
	auto i = tempMap.find(name);
	if (i != tempMap.end())
		i->second->setTemp(temp);
	unlock();
}

void State::removeTemp(std::string name)
{
	wrlock();
	tempMap.erase(name);
	unlock();

}

float State::getTemp(std::string name)
{
	float t = 0.0;

	rdlock();
	auto i = tempMap.find(name);
	if (i != tempMap.end())
		t = i->second->getTemp();
	unlock();

	return t;
}

void State::addOutput(State::Output *output)
{
	wrlock();
	if (outputMap.find(output->getName()) == outputMap.end())
		outputMap.insert(std::make_pair(output->getName(), output));
	else
		printf("trying to add duplicate Output %s\n", output->getName().c_str());
	unlock();
}

void State::updateOutput(std::string name, unsigned value)
{
	rdlock();
	auto i = outputMap.find(name);
	if (i != outputMap.end())
		i->second->setValue(value);
	unlock();
}

void State::removeOutput(std::string name)
{
	wrlock();
	outputMap.erase(name);
	unlock();

}

unsigned State::getOutput(std::string name)
{
	unsigned val = 0;

	rdlock();
	auto i = outputMap.find(name);
	if (i != outputMap.end())
		val = i->second->getValue();
	unlock();

	return val;
}


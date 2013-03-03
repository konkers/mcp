#include "State.hpp"

State State::state;

State::State()
{

}

State::~State()
{
	for (auto i: tempSensorMap) {
		delete i.second;
	}
}

void State::addTempSensor(std::string name)
{
	Temp *temp;

	wrlock();
	if (tempSensorMap.find(name) == tempSensorMap.end()) {
		temp = new Temp(name);
		tempSensorMap.insert(std::make_pair(name, temp));
	}
	unlock();
}

void State::updateTempSensor(std::string name, float temp)
{
	rdlock();
	auto i = tempSensorMap.find(name);
	if (i != tempSensorMap.end())
		i->second->setTemp(temp);
	unlock();
}

void State::removeTempSensor(std::string name)
{
	wrlock();
	tempSensorMap.erase(name);
	unlock();

}

float State::getTemp(std::string name)
{
	float t = 0.0;

	rdlock();
	auto i = tempSensorMap.find(name);
	if (i != tempSensorMap.end())
		t = i->second->getTemp();
	unlock();

	return t;
}

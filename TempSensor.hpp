#ifndef __TEMPSENSOR_HPP__
#define __TEMPSENSOR_HPP__

#include <string>

class TempSensor {
public:
	virtual float getTemp(void) = 0;
	virtual const std::string getName(void) = 0;
};

#endif /* __TEMPSENSOR_HPP__ */

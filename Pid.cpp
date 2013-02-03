#include "Pid.hpp"

Pid::Pid(float setPoint, float p, float i, float d):
	setPoint(setPoint), p(p), i(i), d(d)
{
	reset();
}

void Pid::reset(void)
{
	pkt_1 = 0.0;
	ekt_1 = 0.0;
}

// PID algorithm from "Microcontroller Based Temperature Monitoring and Control"
// by Dogan Ibrahim page 218
float Pid::update(float curTemp)
{
	// Calculate Error
	float ekt = setPoint - curTemp;

	// Calculate I term
	float pkt = i * ekt + pkt_1;

	// Calculate D term
	float qkt = d * (ekt - ekt_1);

	// Calculate PID output
	float ukt = pkt + p * ekt + qkt;

	// Control for integral windup
	if (ukt > maxUkt) {
		pkt = pkt_1;
		ukt = maxUkt;
	} else if (ukt < minUkt) {
		pkt = pkt_1;
		ukt = minUkt;
	}

	pkt_1 = pkt;
	ekt_1 = ekt;
	this->curTemp = curTemp;
	return ukt / maxUkt;
}

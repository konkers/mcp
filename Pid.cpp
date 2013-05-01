// Copyright 2013 Erik Gilling <konkers@konkers.net>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the Licene.

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

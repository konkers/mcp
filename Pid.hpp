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

#ifndef __PID_HPP__
#define __PID_HPP__

#include <stdint.h>
#include <sys/time.h>

class Pid {
private:
	float setPoint;
	float curTemp;
	float p;
	float i;
	float d;

	float pkt_1;
	float ekt_1;

	unsigned curEktCycles;
	unsigned overEktCycles;
	unsigned underEktCycles;

	const float maxUkt = 100.0;
	const float minUkt = 0.0;

        struct timeval  lastUpdateTime;

public:
	Pid(float setPoint, float p, float i, float d);

	void reset(void);
	float update(float curTemp);

	float getCurTemp(void) {
		return curTemp;
	}

	float getSetPoint(void) {
		return setPoint;
	}

	void setSetPoint(float setPoint) {
		this->setPoint = setPoint;
	}

	float getP(void) {
		return p;
	}

	void setP(float p) {
		this->p = p;
	}

	float getI(void) {
		return i;
	}

	void setI(float i) {
		this->i = i;
	}

	float getD(void) {
		return d;
	}

	void setD(float d) {
		this->d = d;
	}

	float getPkt_1(void) {
		return pkt_1;
	}

	float getEkt_1(void) {
		return ekt_1;
	}

	unsigned getOverCycles(void) {
		return overEktCycles;
	}

	unsigned getUnderCycles(void) {
		return underEktCycles;
	}

        unsigned getSecondsSinceUpdate(void) {
            struct timeval now;
            struct timeval diff;

            gettimeofday(&now, NULL);
            timersub(&now, &lastUpdateTime, &diff);

            return diff.tv_sec;
        }
};

#endif /* __PID_HPP__ */

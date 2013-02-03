#ifndef __PID_HPP__
#define __PID_HPP__

class Pid {
private:
	float setPoint;
	float curTemp;
	float p;
	float i;
	float d;

	float pkt_1;
	float ekt_1;

	const float maxUkt = 100.0;
	const float minUkt = 0.0;

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
};

#endif /* __PID_HPP__ */

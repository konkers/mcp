package main

type PidController struct {
	input  Input
	output Output
	p      float32
	i      float32
	d      float32

	setPoint float32
	curTemp  float32

	pkt_1 float32
	ekt_1 float32
}

func NewPidController(input Input, output Output,
	p float32, i float32, d float32, setPoint float32) (controller Controller) {

	pid := &PidController{
		input:    input,
		output:   output,
		p:        p,
		i:        i,
		d:        d,
		setPoint: setPoint,
		curTemp:  0.0,
		pkt_1:    0.0,
		ekt_1:    0.0,
	}

	return pid
}

// PID algorithm from "Microcontroller Based Temperature Monitoring and Control"
// by Dogan Ibrahim page 218
func (pid *PidController) Sync() {
	if !pid.input.HasNewValue() {
		return
	}

	pid.curTemp = pid.input.GetValue()

	// Caclulate Error
	ekt := pid.setPoint - pid.curTemp

	// Calculate I term
	pkt := pid.i*ekt + pid.pkt_1

	// Calculate D term
	qkt := pid.d * (ekt - pid.ekt_1)

	// Calculate PID output
	ukt := pkt + pid.p*ekt + qkt

	// Controll fro intergral windup
	if ukt > 100.0 {
		pkt = pid.pkt_1
		ukt = 100.0
	} else if ukt < 0.0 {
		pkt = pid.pkt_1
		ukt = 0.0
	}

	pid.pkt_1 = pkt
	pid.ekt_1 = ekt

	pid.output.SetValue(ukt / 100.0)
}

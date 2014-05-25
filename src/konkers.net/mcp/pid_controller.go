package main

import ()

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

	params map[string]float32
}

func NewPidController(config *ControllerConfig, state *State) (controller Controller, err error) {

	input, err := state.GetInput(config.Pid.Input)
	if err != nil {
		return nil, err
	}
	output, err := state.GetOutput(config.Pid.Output)
	if err != nil {
		return nil, err
	}

	pid := &PidController{
		input:    input,
		output:   output,
		p:        config.Pid.P,
		i:        config.Pid.I,
		d:        config.Pid.D,
		setPoint: 0.0,
		curTemp:  0.0,
		pkt_1:    0.0,
		ekt_1:    0.0,
	}

	pid.params = make(map[string]float32)

	pid.params["p"] = pid.p
	pid.params["i"] = pid.i
	pid.params["d"] = pid.d
	pid.params["set_point"] = pid.setPoint
	pid.params["cur_temp"] = pid.curTemp

	pid.params["pkt_1"] = pid.pkt_1
	pid.params["ekt_1"] = pid.ekt_1

	state.AddParameterGroup(config.Name, &pid.params)
	return pid, nil
}

func (pid *PidController) checkNewParams() {
	if pid.p != pid.params["p"] ||
		pid.i != pid.params["i"] ||
		pid.d != pid.params["d"] ||
		pid.setPoint != pid.params["set_point"] {

		pid.p = pid.params["p"]
		pid.i = pid.params["i"]
		pid.d = pid.params["d"]
		pid.setPoint = pid.params["set_point"]

		pid.pkt_1 = 0.0
		pid.ekt_1 = 0.0
	}

}

// PID algorithm from "Microcontroller Based Temperature Monitoring and Control"
// by Dogan Ibrahim page 218
func (pid *PidController) Sync() {
	if !pid.input.HasNewValue() {
		return
	}

	pid.checkNewParams()

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

	pid.params["cur_temp"] = pid.curTemp
	pid.params["pkt_1"] = pid.pkt_1
	pid.params["ekt_1"] = pid.ekt_1
}

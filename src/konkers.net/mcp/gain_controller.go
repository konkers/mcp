package main

type GainController struct {
	input  Input
	output Output
	gain   float32
}

func NewGainController(input Input, output Output, gain float32) (controller Controller) {
	c := new(GainController)
	c.input = input
	c.output = output
	c.gain = gain

	return c
}

func (c *GainController) Sync() {
	c.output.SetValue(c.input.GetValue() * c.gain)
}

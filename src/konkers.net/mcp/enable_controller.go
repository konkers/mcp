package main

type enableController struct {
	input  Input
	enable Input
	output Output
}

func NewEnableController(input Input, enable Input, output Output) (controller Controller) {
	c := &enableController{
		input:  input,
		enable: enable,
		output: output,
	}

	return c
}

func (c *enableController) Sync() {
	if c.enable.GetValue() > 0.5 {
		c.output.SetValue(c.input.GetValue())
	} else {
		c.output.SetValue(0.0)
	}
}

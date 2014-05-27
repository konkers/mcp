package main

type ParameterController struct {
	output Output

	params map[string]float32
}

func NewParameterController(config *ControllerConfig, state *State) (controller Controller, err error) {
	output, err := state.GetOutput(config.Parameter.Output)
	if err != nil {
		return nil, err
	}

	c := &ParameterController{
		output: output,
	}
	c.params = make(map[string]float32)
	c.params["control"] = 0.0

	state.AddParameterGroup(config.Name, &c.params)

	return c, nil
}

func (c *ParameterController) Sync() {
	c.output.SetValue(c.params["control"])
}

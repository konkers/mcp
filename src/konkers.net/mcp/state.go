package main

import (
	"fmt"
)

type Input interface {
	GetName() (name string)
	GetValue() (value float32)
	HasNewValue() bool
}

type Output interface {
	GetName() (name string)
	GetValue() (value float32)
	SetValue(value float32)
	HasNewValue() bool
}

type State struct {
	inputs  map[string]Input
	outputs map[string]Output
}

func NewState() (state *State) {
	state = new(State)
	state.inputs = make(map[string]Input)
	state.outputs = make(map[string]Output)

	return state
}

func (state *State) AddInput(input Input) (err error) {
	name := input.GetName()

	if _, ok := state.inputs[name]; ok {
		return fmt.Errorf("Duplicate input name %s", name)
	}

	state.inputs[name] = input
	return nil
}

func (state *State) AddOutput(output Output) (err error) {
	name := output.GetName()

	if _, ok := state.outputs[name]; ok {
		return fmt.Errorf("Duplicate output name %s", name)
	}

	state.outputs[name] = output
	return nil
}

func (state *State) GetInput(name string) (input Input, err error) {
	if input, ok := state.inputs[name]; ok {
		return input, nil
	}

	if input, ok := state.outputs[name]; ok {
		return input, nil
	}
	return nil, fmt.Errorf("Can't find input %s", name)
}

func (state *State) GetOutput(name string) (output Output, err error) {
	if _, ok := state.outputs[name]; !ok {
		return nil, fmt.Errorf("Can't find output %s", name)
	}

	return state.outputs[name], nil
}

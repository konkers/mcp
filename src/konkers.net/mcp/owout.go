package main

import (
	"time"
)

type owOutPort struct {
	value float32
	index byte
	name  string
}

type owOut struct {
	address Address
	bus     Bus
	config  *OwoutConfig

	outputs []*owOutPort
}

func NewOwOut(bus Bus, address Address, config *OwoutConfig) (device Device, err error) {

	out := &owOut{
		address: address,
		bus:     bus,
		config:  config,
	}

	out.outputs = make([]*owOutPort, len(config.Ports))

	var numOuts uint = 0

	for _, port := range config.Ports {
		outPort := new(owOutPort)
		outPort.index = port.Position
		outPort.name = port.Name
		outPort.value = 0.0

		out.outputs[numOuts] = outPort
		numOuts++
	}
	return out, nil
}

func (out *owOut) GetOutputs() []Output {
	outputs := make([]Output, len(out.outputs))
	for i, output := range out.outputs {
		outputs[i] = output
	}
	return outputs
}

func (out *owOut) GetInputs() []Input {
	return make([]Input, 0)
}

func (out *owOut) Sync() (err error) {
	bus := out.bus

	nowValue := float32(time.Now().Nanosecond()) / 999999999.0

	_, err = bus.Reset()
	if err != nil {
		return err
	}

	err = bus.MatchRom(out.address)
	if err != nil {
		return err
	}

	var outData [2]byte
	outData[0] = 0x4e
	for _, port := range out.outputs {
		if nowValue < port.value {
			outData[1] |= 1 << port.index
		}
	}
	for _, value := range outData {
		err = bus.WriteByte(value)
		if err != nil {
			return err
		}
	}

	return nil
}

func (port *owOutPort) GetValue() (value float32) {
	return port.value
}

func (port *owOutPort) SetValue(value float32) {
	port.value = value
}

func (port *owOutPort) HasNewValue() bool {
	return false
}

func (port *owOutPort) GetName() (name string) {
	return port.name
}

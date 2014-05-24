package main

import (
	"fmt"
)

type OwDioPort struct {
	value float32
	index byte
	name  string
}

type OwDio struct {
	address Address
	bus     Bus
	config  *OwioConfig

	inputs  []*OwDioPort
	outputs []*OwDioPort

	directionConfig byte
}

func NewOwDio(bus Bus, address Address, config *OwioConfig) (owdio *OwDio, err error) {

	dio := &OwDio{
		address: address,
		bus:     bus,
		config:  config,
	}
	var numIns uint = 0
	var numOuts uint = 0

	// first pass counts number of input and output ports
	for _, port := range config.Ports {
		if port.Direction == "out" {
			numOuts++
		} else if port.Direction == "in" {
			numIns++
		}
	}

	dio.inputs = make([]*OwDioPort, numIns)
	dio.outputs = make([]*OwDioPort, numOuts)

	numIns = 0
	numOuts = 0

	// second pass creates the ports
	for _, port := range config.Ports {
		dioPort := new(OwDioPort)
		dioPort.index = port.Position
		dioPort.name = port.Name

		if port.Direction == "out" {
			dio.directionConfig |= 1 << dioPort.index
			dio.outputs[numOuts] = dioPort
			numOuts++
		} else if port.Direction == "in" {
			fmt.Printf("addin in%d %s @ %d\n",
				numIns, dioPort.name, dioPort.index)
			dio.inputs[numIns] = dioPort
			numIns++
		}
	}
	return dio, nil
}

func (dio *OwDio) GetOutputs() []Output {
	outputs := make([]Output, len(dio.outputs))
	for i, output := range dio.outputs {
		outputs[i] = output
	}
	return outputs
}

func (dio *OwDio) GetInputs() []Input {
	inputs := make([]Input, len(dio.inputs))
	for i, input := range dio.inputs {
		inputs[i] = input
	}
	return inputs
}

func (dio *OwDio) Sync() (err error) {
	bus := dio.bus

	_, err = bus.Reset()
	if err != nil {
		return err
	}

	err = bus.MatchRom(dio.address)
	if err != nil {
		return err
	}

	var outData [10]byte
	outData[0] = 0x4e
	for _, port := range dio.outputs {
		outData[1+port.index] = byte(port.value * 255)
	}
	for _, port := range dio.inputs {
		// enable pullups on inputs
		outData[1+port.index] = 0xff
	}
	outData[9] = dio.directionConfig

	for _, value := range outData {
		err = bus.WriteByte(value)
		if err != nil {
			return err
		}
	}

	fmt.Printf("%v\n", outData)

	_, err = bus.Reset()
	if err != nil {
		return err
	}

	err = bus.MatchRom(dio.address)
	if err != nil {
		return err
	}

	err = bus.WriteByte(0xbe)
	if err != nil {
		return err
	}

	data, err := bus.ReadByte()
	if err != nil {
		return err
	}

	fmt.Printf("data: %02x\n", data)
	for _, port := range dio.inputs {
		if data&(1<<port.index) != 0 {
			port.value = 1.0
		} else {
			port.value = 0.0
		}
	}

	return nil
}

func (port *OwDioPort) GetValue() (value float32) {
	return port.value
}

func (port *OwDioPort) SetValue(value float32) {
	port.value = value
}

func (port *OwDioPort) GetName() (name string) {
	return port.name
}

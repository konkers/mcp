package main

import (
	_ "fmt"
)

const (
	DS_STATE_IDLE     = iota
	DS_STATE_SAMPLING = iota
	DS_STATE_SAMPLED  = iota
)

const (
	DS18B20_CMD_CONVERT_T        = 0x44
	DS18B20_CMD_WRITE_SCRATCHPAD = 0x4e
	DS18B20_CMD_READ_SCRATCHPAD  = 0xbe
	DS18B20_CMD_COPY_SCRATCHPAD  = 0x48
	DS18B20_CMD_RECAL_E2         = 0xb8
	DS18B20_CMD_READ_PS          = 0xb4
)

var ds18b20HasMaster bool = false
var ds18b20State int = DS_STATE_IDLE

type ds18b20 struct {
	address Address
	bus     Bus

	value float32
	name  string

	offset float32
	scale  float32

	hasNewValue bool
	isMaster    bool
	iter        uint
}

func NewDs18b20(bus Bus, address Address, config *Ds18b20Config) (device Device, err error) {

	ds := &ds18b20{
		address:     address,
		bus:         bus,
		value:       0.0,
		name:        config.Name,
		offset:      -config.Calibration0,
		scale:       100.0 / (config.Calibration100 - config.Calibration0),
		hasNewValue: false,
		isMaster:    false,
	}

	if ds18b20HasMaster == false {
		ds.isMaster = true
		ds18b20HasMaster = true
	}

	return ds, nil
}

func (ds *ds18b20) GetOutputs() []Output {
	return make([]Output, 0)
}

func (ds *ds18b20) GetInputs() []Input {
	inputs := make([]Input, 1)
	inputs[0] = ds
	return inputs
}

func (ds *ds18b20) handleIdleState() (state int, err error) {
	_, err = ds.bus.Reset()
	if err != nil {
		return ds18b20State, err
	}

	err = ds.bus.SkipRom()
	if err != nil {
		return ds18b20State, err
	}

	err = ds.bus.WriteByte(DS18B20_CMD_CONVERT_T)
	if err != nil {
		return ds18b20State, err
	}
	ds.iter = 0

	return DS_STATE_SAMPLING, nil
}

func (ds *ds18b20) handleSamplingState() (state int, err error) {
	busState, err := ds.bus.Read()
	if err != nil {
		return ds18b20State, err
	}
	ds.iter++

	if busState == 1 {
		return DS_STATE_SAMPLED, nil
	} else {
		return DS_STATE_SAMPLING, nil
	}
}

func (ds *ds18b20) handleSampledState() (state int, err error) {
	return DS_STATE_IDLE, nil
}

func (ds *ds18b20) Sync() (err error) {
	ds.hasNewValue = false
	if ds.isMaster {
		switch ds18b20State {
		case DS_STATE_IDLE:
			ds18b20State, err = ds.handleIdleState()

		case DS_STATE_SAMPLING:
			ds18b20State, err = ds.handleSamplingState()

		case DS_STATE_SAMPLED:
			ds18b20State, err = ds.handleSampledState()
		}
		if err != nil {
			return err
		}
	}

	if ds18b20State == DS_STATE_SAMPLED {
		_, err = ds.bus.Reset()
		if err != nil {
			return err
		}

		err = ds.bus.MatchRom(ds.address)
		if err != nil {
			return err
		}

		err = ds.bus.WriteByte(DS18B20_CMD_READ_SCRATCHPAD)
		if err != nil {
			return err
		}

		var highByte uint8
		var lowByte uint8

		lowByte, err = ds.bus.ReadByte()
		if err != nil {
			return err
		}

		highByte, err = ds.bus.ReadByte()
		if err != nil {
			return err
		}

		var intValue int16 = int16(highByte)<<8 + int16(lowByte)

		newTemp := float32(intValue) / 16.0
		newTemp -= ds.offset
		newTemp *= ds.scale

		ds.value = newTemp
		ds.hasNewValue = true
	}
	return nil
}

func (ds *ds18b20) GetValue() (value float32) {
	return ds.value
}

func (ds *ds18b20) HasNewValue() bool {
	return ds.hasNewValue
}

func (ds *ds18b20) GetName() (name string) {
	return ds.name
}

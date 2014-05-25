package main

import (
	"errors"
	"fmt"

	"github.com/kylelemons/gousb/usb"
)

type Address struct {
	Address [8]byte
}

type Bus interface {
	Reset() (state byte, err error)
	MatchRom(address Address) (err error)
	SkipRom() (err error)
	Read() (state byte, err error)
	ReadByte() (data byte, err error)
	WriteByte(data byte) (err error)
	Enumerate() (addresses []Address, err error)
}

type Device interface {
	Sync() (err error)
	GetOutputs() []Output
	GetInputs() []Input
}

type Dongle interface {
	GetBus(index byte) (bus Bus, err error)
	Close()
}

type OwDongleBus struct {
	index  byte
	dongle *OwDongle
}

type OwDongle struct {
	context     *usb.Context
	device      *usb.Device
	inEndpoint  usb.Endpoint
	outEndpoint usb.Endpoint
	buses       [2]OwDongleBus
}

const (
	OW_ENUMERATE  = 0x0
	OW_RESET      = 0x1
	OW_MATCH_ROM  = 0x2
	OW_SKIP_ROM   = 0x3
	OW_READ       = 0x4
	OW_READ_BYTE  = 0x5
	OW_WRITE_BYTE = 0x6
)

func NewAddress(addressString string) (address Address, err error) {
	_, err = fmt.Sscanf(addressString, "%x-%x-%x-%x-%x-%x-%x-%x",
		&address.Address[0], &address.Address[1],
		&address.Address[2], &address.Address[3],
		&address.Address[4], &address.Address[5],
		&address.Address[6], &address.Address[7])
	return address, err
}

func DongleOpen() (dongle Dongle, err error) {

	var dongle_index int = 0

	context := usb.NewContext()
	defer func() {
		if err != nil {
			context.Close()
		}
	}()

	devs, err := context.ListDevices(func(desc *usb.Descriptor) bool {
		if desc.Vendor == 0x18d1 && desc.Product == 0xbeef {
			return true
		}

		return false
	})

	if err != nil {
		for _, d := range devs {
			d.Close()
		}
		return nil, err
	}

	if len(devs) == 0 {
		return nil, errors.New("can't find dongle")
	}

	owdongle := new(OwDongle)
	owdongle.context = context
	for index, dev := range devs {
		if index == dongle_index {
			owdongle.device = dev
		} else {
			dev.Close()
		}
	}
	defer func() {
		if err != nil {
			owdongle.device.Close()
		}
	}()

	desc := owdongle.device.Descriptor
	endpoints := desc.Configs[0].Interfaces[0].Setups[0].Endpoints
	for _, endpoint := range endpoints {
		if endpoint.Attributes&uint8(usb.TRANSFER_TYPE_MASK) != uint8(usb.TRANSFER_TYPE_BULK) {
			continue
		}
		dir := endpoint.Address & uint8(usb.ENDPOINT_DIR_MASK)
		if dir == uint8(usb.ENDPOINT_DIR_IN) {
			owdongle.inEndpoint, _ = owdongle.device.OpenEndpoint(1, 0, 0, endpoint.Address)
		} else {
			owdongle.outEndpoint, _ = owdongle.device.OpenEndpoint(1, 0, 0, endpoint.Address)
		}
	}
	owdongle.buses[0].index = 0
	owdongle.buses[0].dongle = owdongle
	owdongle.buses[1].index = 1
	owdongle.buses[1].dongle = owdongle

	return owdongle, nil
}

func (dongle *OwDongle) GetBus(index byte) (bus Bus, err error) {
	if index > 1 {
		return nil, errors.New("Bus index > 1")
	}

	return &dongle.buses[index], nil
}

func (dongle *OwDongle) Close() {
	dongle.device.Close()
	dongle.context.Close()

}

func (dongle *OwDongle) doCommand(command []byte, response []byte) (responseLen int, err error) {
	_, err = dongle.outEndpoint.Write(command)
	if err != nil {
		return 0, err
	}

	var readLen int = 0
	if len(response) > 0 {
		readLen, err = dongle.inEndpoint.Read(response)
		if err != nil {
			return 0, err
		}
	}

	return readLen, err
}

func (dongle *OwDongle) doShortCommand(command byte, bus byte,
	response []byte) (responseLen int, err error) {

	commandBuff := []byte{command, bus}

	return dongle.doCommand(commandBuff, response)
}

func (bus *OwDongleBus) Reset() (state byte, err error) {
	response := make([]byte, 1)
	_, err = bus.dongle.doShortCommand(OW_RESET, bus.index, response)

	return response[0], err
}

func (bus *OwDongleBus) MatchRom(addr Address) (err error) {
	command := make([]byte, 10)
	command[0] = OW_MATCH_ROM
	command[1] = bus.index
	copy(command[2:10], addr.Address[0:8])
	_, err = bus.dongle.doCommand(command, make([]byte, 0))

	return err
}

func (bus *OwDongleBus) SkipRom() (err error) {
	_, err = bus.dongle.doShortCommand(OW_SKIP_ROM, bus.index, make([]byte, 0))
	return err
}

func (bus *OwDongleBus) Read() (state byte, err error) {
	response := make([]byte, 1)
	_, err = bus.dongle.doShortCommand(OW_READ, bus.index, response)

	return response[0], err
}

func (bus *OwDongleBus) ReadByte() (data byte, err error) {
	response := make([]byte, 1)
	_, err = bus.dongle.doShortCommand(OW_READ_BYTE, bus.index, response)

	return response[0], err
}

func (bus *OwDongleBus) WriteByte(data byte) (err error) {
	command := []byte{OW_WRITE_BYTE, bus.index, data}
	_, err = bus.dongle.doCommand(command, make([]byte, 0))

	return err
}

func (bus *OwDongleBus) Enumerate() ([]Address, error) {
	response := make([]byte, 4096)
	responseLength, err := bus.dongle.doShortCommand(OW_ENUMERATE, bus.index, response)
	if err != nil {
		return nil, err
	}

	responseLength -= responseLength % 8

	addrs := make([]Address, responseLength/8)

	for i := 0; i < responseLength; i += 8 {
		copy(addrs[i/8].Address[:], response[i:i+8])
	}

	return addrs, nil
}

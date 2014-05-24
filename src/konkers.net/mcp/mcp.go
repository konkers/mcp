package main

import (
	"fmt"
)

func main() {
	configDb, err := NewConfigDb("config.json")
	if err != nil {
		panic(err)
	}

	state := NewState()

	dongle, err := DongleOpen()
	if err != nil {
		panic(err)
	}

	devices := make(map[Address]Device)

	for i := byte(0); i < 2; i++ {
		bus, _ := dongle.GetBus(i)

		addrs, err := bus.Enumerate()
		if err != nil {
			panic(err)
		}

		for _, addr := range addrs {
			fmt.Printf("%d: %v\n", i, addr.Address)
			config, err := configDb.LookupDevice(addr)
			if err != nil {
				panic(err)
			}

			if config.Owio != nil {
				owdio, err := NewOwDio(bus, addr, config.Owio)
				if err != nil {
					panic(err)
				}
				devices[addr] = owdio
				for _, out := range owdio.GetOutputs() {
					err = state.AddOutput(out)
					if err != nil {
						panic(err)
					}

				}
				for _, in := range owdio.GetInputs() {
					err = state.AddInput(in)
					if err != nil {
						panic(err)
					}

				}

			}

		}
	}

	hltSwitch, err := state.GetInput("hlt_switch")
	if err != nil {
		panic(err)
	}
	rimsLed, err := state.GetOutput("rims_led")
	if err != nil {
		panic(err)
	}

	for {
		for _, device := range devices {
			err := device.Sync()
			if err != nil {
				panic(err)
			}
		}
		rimsLed.SetValue(hltSwitch.GetValue())
	}

	dongle.Close()
}

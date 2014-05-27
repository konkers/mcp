package main

import (
	"fmt"
	"log"
	"time"
)

type Controller interface {
	Sync()
}

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
			fmt.Printf("%d: %v\n", i, addr)
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

			} else if config.Owout != nil {
				dev, err := NewOwOut(bus, addr, config.Owout)
				if err != nil {
					panic(err)
				}
				devices[addr] = dev

			} else if config.Ds18b20 != nil {
				dev, err := NewDs18b20(bus, addr, config.Ds18b20)
				if err != nil {
					panic(err)
				}
				devices[addr] = dev

			}

		}
	}
	for _, dev := range devices {
		for _, out := range dev.GetOutputs() {
			err = state.AddOutput(out)
			if err != nil {
				panic(err)
			}

		}
		for _, in := range dev.GetInputs() {
			err = state.AddInput(in)
			if err != nil {
				panic(err)
			}

		}
	}

	controllers := make(map[string]Controller)
	for _, c := range configDb.GetConfig().Controllers {
		if c.Gain != nil {
			input, err := state.GetInput(c.Gain.Input)
			if err != nil {
				panic(err)
			}
			output, err := state.GetOutput(c.Gain.Output)
			if err != nil {
				panic(err)
			}
			controllers[c.Name] = NewGainController(input, output, c.Gain.Gain)
		} else if c.Enable != nil {
			input, err := state.GetInput(c.Enable.Input)
			if err != nil {
				panic(err)
			}
			enable, err := state.GetInput(c.Enable.Enable)
			if err != nil {
				panic(err)
			}
			output, err := state.GetOutput(c.Enable.Output)
			if err != nil {
				panic(err)
			}
			controllers[c.Name] = NewEnableController(input, enable, output)
		} else if c.Pid != nil {
			controller, err := NewPidController(c, state)
			if err != nil {
				panic(err)
			}
			controllers[c.Name] = controller
		} else if c.Parameter != nil {
			controller, err := NewParameterController(c, state)
			if err != nil {
				panic(err)
			}
			controllers[c.Name] = controller
		}
	}

	//pidIn, _ := state.GetInput("rims")
	//pidOut, _ := state.GetOutput("rims_led")
	//controllers["pid"] = NewPidController(pidIn, pidOut, 90.0, 9.0, 3.1, 30.0)

	go Webserver(state)

	start := time.Now()
	cyclePeriod := time.Millisecond * 100
	for {
		state.Lock()
		for _, device := range devices {
			err := device.Sync()
			if err != nil {
				panic(err)
			}
		}
		for _, controller := range controllers {
			controller.Sync()
		}
		state.Unlock()

		end := time.Now()
		start = start.Add(cyclePeriod)
		for start.Before(end) {
			log.Printf("cycled overflow\n")
			start = start.Add(cyclePeriod)
		}

		// as far as I can tell the Go time APIs do not provide
		// a mecahnism for absolute sleeps so we'll fake it with
		// a relative one
		time.Sleep(start.Sub(time.Now()))
	}

	dongle.Close()
}

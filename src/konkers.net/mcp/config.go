package main

import (
	"bufio"
	"encoding/json"
	"errors"
	"fmt"
	"io/ioutil"
	"os"
	"strings"
)

type Ds18B20Config struct {
	Name           string  `json:"name"`
	Calibration100 float32 `json:"cal100"`
	Calibration0   float32 `json:"cal0"`
}

type OwoutPortConfig struct {
	Name     string `json:"name"`
	Position uint8  `json:"position"`
}

type OwoutConfig struct {
	Ports []OwoutPortConfig `json:"ports"`
}

type OwioPortConfig struct {
	Name      string `json:"name"`
	Direction string `json:"direction"`
	Position  uint8  `json:"position"`
}

type OwioConfig struct {
	Ports []OwioPortConfig `json:"ports"`
}

type DeviceConfig struct {
	Address string         `json:"addr"`
	Ds18B20 *Ds18B20Config `json:"ds18b20,omitempty"`
	Owout   *OwoutConfig   `json:"owout,omitempty"`
	Owio    *OwioConfig    `json:"owdio,omitempty"`
}

type Config struct {
	Devices []*DeviceConfig `json:"devices"`
}

type ConfigDb struct {
	config     Config
	addressMap map[Address]*DeviceConfig
}

func formatSyntaxError(js string, err error) (errString string) {
	syntax, ok := err.(*json.SyntaxError)
	if !ok {
		fmt.Println(err)
		return ""
	}

	start, end := strings.LastIndex(js[:syntax.Offset], "\n")+1, len(js)
	if idx := strings.Index(js[start:], "\n"); idx >= 0 {
		end = start + idx
	}
	line, pos := strings.Count(js[:start], "\n"), int(syntax.Offset)-start-1

	errString = fmt.Sprintf("Error in line %d: %s \n", line, err)
	errString += fmt.Sprintf("%s\n%s^", js[start:end], strings.Repeat(" ", pos))
	return errString
}

func NewConfigDb(filename string) (configDb *ConfigDb, err error) {
	configDb = new(ConfigDb)

	file, err := os.Open(filename)
	if err != nil {
		return nil, err
	}
	defer file.Close()

	buffer, err := ioutil.ReadAll(bufio.NewReader(file))
	if err != nil {
		return nil, err
	}

	err = json.Unmarshal(buffer, &configDb.config)
	if err != nil {
		return nil, errors.New(formatSyntaxError(string(buffer), err))
	}

	configDb.addressMap = make(map[Address]*DeviceConfig)
	for _, dev := range configDb.config.Devices {
		address, err := NewAddress(dev.Address)
		if err != nil {
			return nil, fmt.Errorf("Can't parse address \"%s\"", dev.Address)
		}
		configDb.addressMap[address] = dev
	}

	//	b, err := json.Marshal(configDb.config)
	//	if err != nil {
	//		fmt.Println("error:", err)
	//	}
	//	os.Stdout.Write(b)

	return configDb, nil
}

func (db *ConfigDb) LookupDevice(address Address) (device *DeviceConfig, err error) {
	device, ok := db.addressMap[address]
	if !ok {
		return nil, fmt.Errorf("Can't find address %s in config db", address)
	}
	return device, nil
}

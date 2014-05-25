package main

import (
	"encoding/json"
	"fmt"
	"net/http"
	"os"
	"regexp"
	"time"
)

type mcpHandler struct {
	state *State

	statusReg *regexp.Regexp
	paramsReg *regexp.Regexp
}

type statusResponse struct {
	Inputs     map[string]float32              `json:"inputs"`
	Outputs    map[string]float32              `json:"outputs"`
	Parameters *map[string]*map[string]float32 `json:"parameters"`
}

func Webserver(state *State) {
	handler := &mcpHandler{
		state:     state,
		statusReg: regexp.MustCompile("^/status$"),
		paramsReg: regexp.MustCompile("^/params/([a-zA-Z0-9_-]+)/(set)$"),
	}
	http.ListenAndServe(":8080", handler)
	i := 0
	for {
		time.Sleep(time.Second)
		state.Lock()
		b, err := json.Marshal(state.GetAllParameterGroups())
		state.Unlock()

		if err == nil {
			os.Stdout.Write(b)
			fmt.Printf("\n")
		}
		i++
		if i == 10 {
			state.Lock()
			group, err := state.GetParameterGroup("rims_pid")
			if err != nil {
				panic(err)
			}
			(*group)["set_point"] = 30.0

			state.Unlock()

		}
	}
}

func (m *mcpHandler) serveStatus(w http.ResponseWriter, req *http.Request, args []string) {

	var resp statusResponse
	resp.Inputs = make(map[string]float32)
	resp.Outputs = make(map[string]float32)

	m.state.Lock()
	resp.Parameters = m.state.GetAllParameterGroups()

	for name, input := range *m.state.GetAllInputs() {
		resp.Inputs[name] = input.GetValue()
	}

	for name, output := range *m.state.GetAllOutputs() {
		resp.Outputs[name] = output.GetValue()
	}

	b, err := json.Marshal(resp)
	m.state.Unlock()
	if err != nil {
		http.NotFound(w, req)
		return
	}

	w.Write(b)

}

func (m *mcpHandler) serveStatus(w http.ResponseWriter, req *http.Request, args []string) {
}

func (m *mcpHandler) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	if args := m.statusReg.FindStringSubmatch(req.URL.Path); args != nil {
		m.serveStatus(w, req, args)
	} else if args := m.paramsReg.FindStringSubmatch(req.URL.Path); args != nil {
		m.serveParams(w, req, args)
	} else {
		http.NotFound(w, req)
	}

}

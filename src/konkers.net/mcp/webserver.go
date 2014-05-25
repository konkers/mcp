package main

import (
	"encoding/json"
	"fmt"
	"io/ioutil"
	"net/http"
	"os"
	"regexp"
	"time"
)

type mcpHandler struct {
	state *State

	statusReg *regexp.Regexp
	paramsReg *regexp.Regexp
	staticReg *regexp.Regexp

	staticHandler http.Handler
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
		staticReg: regexp.MustCompile("^/static/"),
		staticHandler: http.StripPrefix("/static/",
			http.FileServer(http.Dir("./src/konkers.net/mcp/static"))),
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

func (m *mcpHandler) handleStatus(w http.ResponseWriter, req *http.Request, args []string) {

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

func (m *mcpHandler) handleParams(w http.ResponseWriter, req *http.Request, args []string) {
	params := make(map[string]float32)
	body, err := ioutil.ReadAll(req.Body)
	if err != nil {
		http.NotFound(w, req)
		return
	}
	err = json.Unmarshal(body, &params)
	if err != nil {
		http.NotFound(w, req)
		return
	}
	m.state.Lock()
	defer m.state.Unlock()

	groupP, err := m.state.GetParameterGroup(args[1])
	if err != nil {
		http.NotFound(w, req)
		return
	}

	group := *groupP

	// first make sure that all params exist
	for name, _ := range params {
		if _, ok := group[name]; !ok {
			http.NotFound(w, req)
			return
		}
	}

	// then set the values
	for name, value := range params {
		group[name] = value
	}
}

func (m *mcpHandler) ServeHTTP(w http.ResponseWriter, req *http.Request) {
	if args := m.statusReg.FindStringSubmatch(req.URL.Path); args != nil {
		m.handleStatus(w, req, args)
	} else if args := m.paramsReg.FindStringSubmatch(req.URL.Path); args != nil {
		m.handleParams(w, req, args)
	} else if args := m.staticReg.FindStringSubmatch(req.URL.Path); args != nil {
		m.staticHandler.ServeHTTP(w, req)
	} else if req.URL.Path == "/" {
		http.Redirect(w, req, "/static/", http.StatusFound)

	} else {
		http.NotFound(w, req)
	}

}

// Copyright 2013 Erik Gilling <konkers@konkers.net>
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the Licene.

#ifndef __WEBSERVER_HPP__
#define __WEBSERVER_HPP__

#include <string>

#include <lua.hpp>

#include "EventQueue.hpp"
#include "Pid.hpp"

#include "mongoose.h"

class WebServer {
private:
	int port;
	Pid *pid;
	struct mg_context *ctx;

	bool flow;

	std::string errString;

	EventQueue *eventQueue;

	friend void *mg_callback(enum mg_event event, struct mg_connection *conn);
	void *callback(enum mg_event event, struct mg_connection *conn);

	void luaPushSensor(lua_State *L, State::Temp *t);
	void luaPushOutput(lua_State *L, State::Output *o);

	bool handleInitLua(struct mg_connection *conn);
	bool handleNewRequest(struct mg_connection *conn);

public:
	WebServer(int port, Pid *pid, EventQueue *queue);
	~WebServer();

	bool getFlow(void) {
		return flow;
	}
};
#endif /* __WEBSERVER_HPP__ */

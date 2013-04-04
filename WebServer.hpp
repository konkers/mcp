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

	void luaPushTemp(lua_State *L, State::Temp *t);

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

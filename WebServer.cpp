#include <string.h>

#include "State.hpp"
#include "Thread.hpp"
#include "WebServer.hpp"

void *mg_callback(enum mg_event event, struct mg_connection *conn)
{
	const struct mg_request_info *ri = mg_get_request_info(conn);
	WebServer *server = (WebServer *)ri->user_data;
	return server->callback(event, conn);
}

WebServer::WebServer(int port, Pid *pid, EventQueue *queue) :
	port(port), pid(pid), eventQueue(queue)
{
	char port_str[16];

	snprintf(port_str, sizeof(port_str), "%d", port);

	const char *mg_options[] = {
		"listening_ports", port_str,
		"document_root", "html",
		"error_log_file", "www_err.log",
		"index_files", "index.html,index.lp",
		NULL};

	ctx = mg_start(&mg_callback, this, mg_options);

}

WebServer::~WebServer()
{
	if (ctx)
		mg_stop(ctx);
}

void *WebServer::callback(enum mg_event event, struct mg_connection *conn)
{
	switch(event) {
	case MG_INIT_LUA:
		return (void *)handleInitLua(conn);

	case MG_NEW_REQUEST:
		return (void *)handleNewRequest(conn);

	default:
		return NULL;
	}
}

void WebServer::luaPushSensor(lua_State *L, State::Temp *t)
{
	lua_newtable(L);

	lua_pushstring(L, "name");
	lua_pushstring(L, t->getName().c_str());
	lua_settable(L, -3);

	lua_pushstring(L, "temp");
	lua_pushnumber(L, t->getTemp());
	lua_settable(L, -3);

	// HACK! assume PID conection with RIMS
	if(t->getName() == "RIMS") {
		lua_pushstring(L, "pid");
		lua_newtable(L);

		lua_pushstring(L, "set_point");
		lua_pushnumber(L, pid->getSetPoint());
		lua_settable(L, -3);

		lua_pushstring(L, "p");
		lua_pushnumber(L, pid->getP());
		lua_settable(L, -3);

		lua_pushstring(L, "i");
		lua_pushnumber(L, pid->getI());
		lua_settable(L, -3);

		lua_pushstring(L, "d");
		lua_pushnumber(L, pid->getD());
		lua_settable(L, -3);

		lua_pushstring(L, "pkt_1");
		lua_pushnumber(L, pid->getPkt_1());
		lua_settable(L, -3);

		lua_pushstring(L, "ekt_1");
		lua_pushnumber(L, pid->getEkt_1());
		lua_settable(L, -3);

		lua_settable(L, -3);
	}
}

bool WebServer::handleInitLua(struct mg_connection *conn)
{
	const struct mg_request_info *ri = mg_get_request_info(conn);
	State *state = State::getState();

	lua_State *L = (lua_State *) ri->ev_data;

	lua_getglobal(L, "package");
	lua_getfield(L, -1, "path");
	std::string path = lua_tostring(L, -1);
	path.append(";lua/luajson/lua/?.lua;lua/lpeg/?.lua");
	lua_pop(L, 1);
	lua_pushstring(L, path.c_str());
	lua_setfield(L, -2, "path");
	lua_getfield(L, -1, "cpath");
	std::string cpath = lua_tostring(L, -1);
	cpath.append(";out/lib/?.so");
	lua_pop(L, 1);
	lua_pushstring(L, cpath.c_str());
	lua_setfield(L, -2, "cpath");
	lua_pop(L, 1);

	lua_pushnumber(L, pid->getCurTemp());
	lua_setglobal(L, "cur_temp");

	lua_pushnumber(L, pid->getSetPoint());
	lua_setglobal(L, "set_point");

	lua_pushnumber(L, pid->getP());
	lua_setglobal(L, "pid_p");

	lua_pushnumber(L, pid->getI());
	lua_setglobal(L, "pid_i");

	lua_pushnumber(L, pid->getD());
	lua_setglobal(L, "pid_d");

	lua_pushnumber(L, pid->getPkt_1());
	lua_setglobal(L, "pid_pkt_1");

	lua_pushnumber(L, pid->getEkt_1());
	lua_setglobal(L, "pid_ekt_1");

	lua_pushboolean(L, (int) flow);
	lua_setglobal(L, "flow");

	lua_pushstring(L, errString.c_str());
	lua_setglobal(L, "err_string");

	lua_newtable(L);

	state->rdlock();
	auto sensorMap = state->getTempSensorMap();
	int sensorIdx = 1;
	for (auto i : *sensorMap) {
		State::Temp *t = i.second;
		luaPushSensor(L, t);
		lua_rawseti(L, -2, sensorIdx++);
	}
	state->unlock();

	lua_setglobal(L, "sensors");

	return true;
}

bool WebServer::handleNewRequest(struct mg_connection *conn)
{
	const struct mg_request_info *ri = mg_get_request_info(conn);
	char post_data[1024], val[sizeof(post_data)];
	float set_point, p, i, d;
	int post_data_len;

	if (!strcmp(ri->uri, "/pid_update")) {
		mg_printf(conn, "HTTP/1.0 200 OK\r\n"
			"Content-Type: application/json\r\n\r\n");
		mg_printf(conn, "{\"success\": \"yes\"}");
		return true;
	} else if (!strcmp(ri->uri, "/update")) {
		char *endp;
		bool dataValid = true;

		// User has submitted a form, show submitted data and a variable value
		post_data_len = mg_read(conn, post_data, sizeof(post_data));

		errString.erase();

		// Parse form data. input1 and input2 are guaranteed to be NUL-terminated
		mg_get_var(post_data, post_data_len, "set_point", val, sizeof(val));
		set_point = strtof(val, &endp);
		if (endp == val) {
			errString += "can't convert set_point to float\n";
			dataValid = false;
		}

		mg_get_var(post_data, post_data_len, "p_term", val, sizeof(val));
		p = strtof(val, &endp);
		if (endp == val) {
			errString += "can't convert p_term to float\n";
			dataValid = false;
		}

		mg_get_var(post_data, post_data_len, "i_term", val, sizeof(val));
		i = strtof(val, &endp);
		if (endp == val) {
			errString += "can't convert i_term to float\n";
			dataValid = false;
		}

		mg_get_var(post_data, post_data_len, "d_term", val, sizeof(val));
		d = strtof(val, &endp);
		if (endp == val) {
			errString += "can't convert d_term to float\n";
			dataValid = false;
		}

		mg_get_var(post_data, post_data_len, "flow", val, sizeof(val));

		flow = !!strcmp(val, "recirc");

		EventQueue::PidUpdateEvent *event =
			new EventQueue::PidUpdateEvent(set_point, p, i, d);
		eventQueue->postEvent(event);

		const char *host = mg_get_header(conn, "Host");
		mg_printf(conn, "HTTP/1.1 302 Found\r\n"
			  "Location: http://%s/index.lp\r\n\r\n",
			  host);
		return true;
	}
	return false;
}

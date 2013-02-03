#include <string.h>

#include <lua.hpp>

#include "Thread.hpp"
#include "WebServer.hpp"

void *mg_callback(enum mg_event event, struct mg_connection *conn)
{
	const struct mg_request_info *ri = mg_get_request_info(conn);
	WebServer *server = (WebServer *)ri->user_data;
	return server->callback(event, conn);
}

WebServer::WebServer(int port, Pid *pid) :
	port(port), pid(pid)
{
	char port_str[16];

	snprintf(port_str, sizeof(port_str), "%d", port);

	const char *mg_options[] = {
		"listening_ports", port_str,
		"document_root", "html",
		"error_log_file", "www_err.log",
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

bool WebServer::handleInitLua(struct mg_connection *conn)
{
	const struct mg_request_info *ri = mg_get_request_info(conn);

	lua_State *L = (lua_State *) ri->ev_data;

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

	return true;
}

bool WebServer::handleNewRequest(struct mg_connection *conn)
{
	const struct mg_request_info *ri = mg_get_request_info(conn);
	char post_data[1024], val[sizeof(post_data)];
	float p, i, d;
	int post_data_len;

	if (!strcmp(ri->uri, "/pid_update")) {
		char *endp;
		bool dataValid = true;

		// User has submitted a form, show submitted data and a variable value
		post_data_len = mg_read(conn, post_data, sizeof(post_data));

		errString.erase();

		// Parse form data. input1 and input2 are guaranteed to be NUL-terminated
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

		printf("p=%f, i=%f, d=%f\n", p, i, d);

		const char *host = mg_get_header(conn, "Host");
		mg_printf(conn, "HTTP/1.1 302 Found\r\n"
			  "Location: http://%s/test.lp\r\n\r\n",
			  host);
		return true;
	}
	return false;
}
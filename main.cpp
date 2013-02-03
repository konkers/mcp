#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <vector>
#include <map>
using namespace std;

extern "C" {
#include <lua.h>
}

#include "Dongle.hpp"
#include "Ds18b20.hpp"
#include "Pid.hpp"

#include "mongoose.h"

vector<Ds18b20 *> sensors;
map<Dongle::Addr, Ds18b20 *> sensorMap;
Pid pid(79.5, 50.0, 0, 0);
Ds18b20 *heaterTemp = NULL;


// 28 c5 c5 f4 03 00 00 01
//  0.0 = 0.250000
//  100.0 = 100

// 28 77 02 8d 02 00 00 8b
//  0.0 = 0.375000
//  100.0 = 99.562500

// 28 55 33 8d 02 00 00 1a
//  0.0 = 0.250000
//  100.0 = 99.750000

Ds18b20 *newSensor(Dongle *dongle, Dongle::Addr addr)
{
	if (addr == Dongle::Addr(0x28, 0xc5, 0xc5, 0xf4, 0x03, 0x00, 0x00, 0x01))
		return new Ds18b20(dongle, addr, 0.25, 100.0);
	else if (addr ==  Dongle::Addr(0x28, 0x77, 0x02, 0x8d, 0x02, 0x00, 0x00, 0x8b))
		return new Ds18b20(dongle, addr, 0.375, 99.5625);
	else if (addr ==  Dongle::Addr(0x28, 0x55, 0x33, 0x8d, 0x02, 0x00, 0x00, 0x1a))
		return new Ds18b20(dongle, addr, 0.25, 99.75);
	else if (addr.addr[0] == 0x28)
		return new Ds18b20(dongle, addr);
	else
		return NULL;
}

bool running = true;

void signal_handler(int signum)
{
	running = false;
	signal(signum, SIG_DFL);
}

struct mg_context *ctx;
const char *mg_options[] = {
	"listening_ports", "8080",
	"document_root", "html",
	"error_log_file", "www_err.log",
	NULL};

static void *mg_callback(enum mg_event event,
                      struct mg_connection *conn)
{
	const struct mg_request_info *ri = mg_get_request_info(conn);
	char post_data[1024], val[sizeof(post_data)];
//	float p, i, d;
	int post_data_len;

	if (event == MG_INIT_LUA) {
		lua_State *L = (lua_State *) ri->ev_data;

		lua_pushnumber(L, pid.getCurTemp());
		lua_setglobal(L, "cur_temp");

		lua_pushnumber(L, pid.getSetPoint());
		lua_setglobal(L, "set_point");

		lua_pushnumber(L, pid.getP());
		lua_setglobal(L, "pid_p");

		lua_pushnumber(L, pid.getI());
		lua_setglobal(L, "pid_i");

		lua_pushnumber(L, pid.getD());
		lua_setglobal(L, "pid_d");

		lua_pushnumber(L, pid.getPkt_1());
		lua_setglobal(L, "pid_pkt_1");

		lua_pushnumber(L, pid.getEkt_1());
		lua_setglobal(L, "pid_ekt_1");



		printf("lua\n");
	}

	if (event != MG_NEW_REQUEST)
		return NULL;

	if (!strcmp(ri->uri, "/pid_update")) {
		// User has submitted a form, show submitted data and a variable value
		post_data_len = mg_read(conn, post_data, sizeof(post_data));

		// Parse form data. input1 and input2 are guaranteed to be NUL-terminated
		mg_get_var(post_data, post_data_len, "p_term", val, sizeof(val));
		printf("p_term = %s\n", val);

		mg_get_var(post_data, post_data_len, "i_term", val, sizeof(val));
		printf("i_term = %s\n", val);

		mg_get_var(post_data, post_data_len, "d_term", val, sizeof(val));
		printf("d_term = %s\n", val);

		const char *host = mg_get_header(conn, "Host");
		mg_printf(conn, "HTTP/1.1 302 Found\r\n"
			  "Location: http://%s/test.lp\r\n\r\n",
			  host);
	} else if (0) {
		mg_printf(conn, "HTTP/1.0 200 OK\r\n"
//			  "Content-Length: %d\r\n"
			  "Content-Type: text/html\r\n\r\n");
		mg_printf(conn, "<html><body><table>\n");
		for (const auto& ds : sensors) {
			float temp = ds->getTemp();
			Dongle::Addr a = ds->getAddr();
			mg_printf(conn, "<tr>");
			mg_printf(conn, "<td>%02x %02x %02x %02x %02x %02x %02x %02x</td>",
				  a.addr[0], a.addr[1], a.addr[2], a.addr[3],
				  a.addr[4], a.addr[5], a.addr[6], a.addr[7]);
			mg_printf(conn, "<td>%f</td>", temp);
			mg_printf(conn, "</tr>");
		}
		mg_printf(conn, "</table></body></html>");
	}
	return NULL;
//	return (void *)"";  // Mark request as processed
}

void startWebServer(void)
{
	ctx = mg_start(&mg_callback, NULL, mg_options);
}

int main(int argc, char *argv[])
{
	Dongle d;
	int ret;
	int i;

	signal(SIGINT, signal_handler);
	signal(SIGTERM, signal_handler);

	printf("1\n");

	if (!d.connect())
		return 1;

	printf("2\n");

	ret = d.enumerate();
	if (ret < 0)
		return 1;

	printf("3\n");

	for (i = 0; i < ret; i++) {
		Dongle::Addr a = d.getAddr(i);
		Ds18b20 *ds = newSensor(&d, a);
		if (ds) {
			sensors.push_back(ds);
			sensorMap[a] = ds;
		}
	}
	printf("4\n");

	startWebServer();

	map<Dongle::Addr, Ds18b20 *>::const_iterator si =
		sensorMap.find(Dongle::Addr(0x28, 0xc5, 0xc5, 0xf4, 0x03, 0x00, 0x00, 0x01));
	if (si != sensorMap.end())
		heaterTemp = si->second;
	else
		printf("Could not find heater temp probe\n");

	while(running) {
		sensors[0]->startAllConversion();
		while (!sensors[0]->isConversionDone()) { }

#if 1
		for (vector<Ds18b20 *>::iterator ds = sensors.begin();
		     ds != sensors.end();
		     ds++) {
			float temp = (*ds)->getTemp();
			Dongle::Addr a = (*ds)->getAddr();
			printf("%02x %02x %02x %02x %02x %02x %02x %02x = %f\n",
			       a.addr[0], a.addr[1], a.addr[2], a.addr[3],
			       a.addr[4], a.addr[5], a.addr[6], a.addr[7],
			       temp);
		}
#endif
		if (heaterTemp) {
			float temp = heaterTemp->getTemp();
			float power = pid.update(temp);
			uint8_t power_1 = power * 255;
			d.setPower(power_1);
		}
		sleep(10);
	}

	printf("exiting\n");
	mg_stop(ctx);

	return 0;
}

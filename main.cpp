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

#include <getopt.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <vector>
#include <map>
using namespace std;

#include "DongleThread.hpp"
#include "EventQueue.hpp"
#include "Pid.hpp"
#include "SimDongle.hpp"
#include "State.hpp"
#include "TimerThread.hpp"
#include "UsbDongle.hpp"
#include "WebServer.hpp"

Pid pid(0, 90.0, 9.0, 3.1);
bool running = true;

void signal_handler(int signum)
{
    running = false;
    signal(signum, SIG_DFL);
}

void usage(void)
{
    printf("usage: sim <option>\n"
           "options:\n"
           "    --sim        enable simuation mode\n");
}

int main(int argc, char *argv[])
{
    bool sim = false;

    {
        int ch;
        static struct option longopts[] = {
            {"sim",		no_argument,	NULL, 's'},
        };

        while ((ch = getopt_long(argc, argv, "s", longopts, NULL)) != -1)
            switch (ch) {
                case 's':
                    sim = true;
                    break;

                default:
                    usage();
                    return 1;
            }
    }

    State *state = State::getState();

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    EventQueue queue;

    WebServer server(8080, &pid, &queue);
    Dongle *d;
    UsbDongle *usb_dongle;
    if (sim) {
        d = new SimDongle();
    } else {
        usb_dongle = new UsbDongle();
        d = usb_dongle->getBus(0);
    }

    DongleThread dongle(d, &queue);
    TimerThread timer(&queue);

    Dongle::Addr valveAddr = Dongle::Addr(0xe0, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);

    timer.start();
    dongle.start();

    while(running) {
        EventQueue::Event *event = queue.getEvent();
        switch (event->getType()) {
            case EventQueue::Event::Type::tick:
                dongle.startConversion();
                break;

            case EventQueue::Event::Type::pidUpdate:
                {
                    EventQueue::PidUpdateEvent *pidEvent =
                        static_cast<EventQueue::PidUpdateEvent *>(event);
                    printf("pid update %f %f %f %f\n",
                           pidEvent->getSetPoint(),
                           pidEvent->getP(),
                           pidEvent->getI(),
                           pidEvent->getD());
                    pid.setSetPoint(pidEvent->getSetPoint());
                    pid.setP(pidEvent->getP());
                    pid.setI(pidEvent->getI());
                    pid.setD(pidEvent->getD());
                    pid.reset();
                    break;
                }
            case EventQueue::Event::Type::outputUpdate:
                {
                    EventQueue::OutputUpdateEvent *outputEvent =
                        static_cast<EventQueue::OutputUpdateEvent *>(event);
                    state->updateOutput(outputEvent->getName(), outputEvent->getValue());
                }
            case EventQueue::Event::Type::stateUpdate:
                {
                    float temp = state->getTemp("RIMS");
                    printf("temp %f\n", temp);
                    float power = pid.update(temp);
                    uint8_t power_1 = power * 255;
                    dongle.setPower(power_1);

                    dongle.sync();

                    break;
                }

            default:
                printf("unknown event type\n");
                break;
        }

        delete(event);
    }

    printf("exiting\n");
    timer.stop();
    dongle.stop();
    return 0;
}

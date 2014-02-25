// Copyright 2014 Erik Gilling <konkers@konkers.net>
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

#include <signal.h>
#include <stdio.h>
#include <unistd.h>

#include <vector>
#include <map>
using namespace std;

#include "DongleThread.hpp"
#include "Ds18b20.hpp"
#include "UsbDongle.hpp"

bool running = true;


void signal_handler(int signum)
{
    running = false;
    signal(signum, SIG_DFL);
}

Ds18b20 *newSensor(Dongle *dongle, Dongle::Addr addr)
{
    if (addr.addr[0] == 0x28)
        return new Ds18b20(dongle, addr);
    else
        return NULL;
}

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    State *state = State::getState();

    UsbDongle usb_dongle;
    for (int bus = 0; bus < 2; bus++) {
         Dongle *d = usb_dongle.getBus(bus);
         int ret, i;

         printf("1\n");
         if (!d->connect())
         return 1;

         printf("2\n");
         ret = d->enumerate();
         if (ret <= 0) {
         return 1;
         }

         vector<Ds18b20 *> sensors;

         for (i = 0; i < ret; i++) {
         Dongle::Addr a = d->getAddr(i);
         Ds18b20 *ds = newSensor(d, a);
         if (ds) {
         sensors.push_back(ds);
         }
         }

         sensors[0]->startAllConversion();
         while (!sensors[0]->isConversionDone()) {
             usleep(100000);
             if (!running) {
                 break;
             }
         }

         for (auto &sensor : sensors) {
             sensor->updateTemp();
             printf("%s: %f\n", sensor->getAddr().getName().c_str(),
                    sensor->getTemp());
         }
    }

    return 0;
}

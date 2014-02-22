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

#include <unistd.h>

#include "TimerThread.hpp"


TimerThread::TimerThread(EventQueue *queue) :
    Thread(), eventQueue(queue) {

}

TimerThread::~TimerThread()
{

}

int TimerThread::run(void)
{
    while(running) {
        sleep(1); // XXX: convert to new C++ chrono api
        EventQueue::TickEvent *event = new EventQueue::TickEvent();
        eventQueue->postEvent(event);
    }

    return 0;
}

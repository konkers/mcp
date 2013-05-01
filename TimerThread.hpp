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

#ifndef __TIMERTHREAD__HPP__
#define __TIMERTHREAD__HPP__

#include "EventQueue.hpp"
#include "Thread.hpp"

class TimerThread : public Thread {
private:
	EventQueue *eventQueue;

protected:
	virtual int run(void);

public:
	TimerThread(EventQueue *queue);
	virtual ~TimerThread();
};

#endif /* __TIMERTHREAD__HPP__ */

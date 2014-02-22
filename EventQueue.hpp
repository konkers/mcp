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

#ifndef __EVENTQUEUE_HPP__
#define __EVENTQUEUE_HPP__

#include <deque>
#include <string>

#include "Thread.hpp"

class EventQueue {
public:
    class Event {
    public:
        enum class Type {none, tick, stateUpdate, pidUpdate, outputUpdate};

    private:
        Type type;

    protected:
        Event(Type type) : type(type){
        }

    public:
        Type getType(void) {
            return type;
        }
    };

    class TickEvent : public Event {
    public:
        TickEvent() : Event(Type::tick) {
        }
    };

    class StateUpdateEvent : public Event {
    private:
    public:
        StateUpdateEvent() :
            Event(Type::stateUpdate){
            }
    };

    class PidUpdateEvent : public Event {
    private:
        float setPoint, p, i, d;
    public:
        PidUpdateEvent(float setPoint, float p, float i, float d) :
            Event(Type::pidUpdate), setPoint(setPoint), p(p), i(i), d(d) {
            }

        float getSetPoint(void) {
            return setPoint;
        }

        float getP(void) {
            return p;
        }

        float getI(void) {
            return i;
        }

        float getD(void) {
            return d;
        }
    };

    class OutputUpdateEvent : public Event {
    private:
        std::string name;
        unsigned value;
    public:
        OutputUpdateEvent(std::string name, unsigned value) :
            Event(Type::outputUpdate), name(name), value(value){
            }

        std::string getName(void) {
            return name;
        }

        unsigned getValue(void) {
            return value;
        }
    };

private:
    std::deque<Event *> events;
    Thread::Condition eventsCond;

public:
    EventQueue();
    ~EventQueue();

    // both of the below transfer ownership of Event
    Event *getEvent(void);
    void postEvent(Event *event);
};

#endif /* __EVENTQUEUE_HPP__ */

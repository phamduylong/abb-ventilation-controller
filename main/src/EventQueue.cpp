/*
 * EventQueue.cpp
 *
 *  Created on: 29 Sep 2022
 *      Author: marin
 */

#include "EventQueue.h"

EventQueue::EventQueue() {}

EventQueue::~EventQueue() {}

void EventQueue::publish(const Event &e) {
	eq.push(e);
}

Event EventQueue::consume() {
	// should not happen if used properly
	if(eq.empty()) return Event(Event::eCriticalError);
	Event e(eq.front());
	eq.pop();
	return e;
}

bool EventQueue::pending() {
	return !eq.empty();
}
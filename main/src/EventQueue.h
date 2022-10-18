/*
 * EventQueue.h
 *
 *  Created on: 29 Sep 2022
 *      Author: marin
 */

#ifndef EVENTQUEUE_H_
#define EVENTQUEUE_H_

#include <queue>
#include "Event.h"

class EventQueue {
public:
	EventQueue();
	virtual ~EventQueue();
	void publish(const Event &e);
	Event consume();
	bool pending();
private:
	std::queue<Event> eq;
};

#endif /* EVENTQUEUE_H_ */

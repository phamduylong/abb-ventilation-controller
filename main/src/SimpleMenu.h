/*
 * SimpleMenu.h
 *
 *  Created on: 3.2.2016
 *      Author: krl
 */

#ifndef SIMPLEMENU_H_
#define SIMPLEMENU_H_
#include <vector>

#include "MenuItem.h"
#include "LpcUart.h"

class SimpleMenu {
public:
	SimpleMenu();
	virtual ~SimpleMenu();
	void addItem(MenuItem *item);
	void event(MenuItem::menuEvent e);
private:
	std::vector<MenuItem *> items;
	int position;
	LpcUart *lpc;
	int counter;

};

#endif /* SIMPLEMENU_H_ */

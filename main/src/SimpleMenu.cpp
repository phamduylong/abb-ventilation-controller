/*
 * SimpleMenu.cpp
 *
 *  Created on: 3.2.2016
 *      Author: krl
 */

#include "SimpleMenu.h"

SimpleMenu::SimpleMenu() {
	position = 0;
	counter = 0;
}

SimpleMenu::~SimpleMenu() {
	// TODO Auto-generated destructor stub
}

void SimpleMenu::addItem(MenuItem *item) {

	const char* name = item->getName();
	int present = 1;
	for(unsigned int i=0;i<items.size();i++){
			if(items[i]->getName() == name){
				present -=1;
				break;
			}
		}
	if(present){
		items.push_back(item);
	}

}

void SimpleMenu::deleteItem(MenuItem *item) {
	const char* name = item->getName();

	for(unsigned int i=0;i<items.size();++i){
		if(name == items[i]->getName()){
			items.erase(next(begin(items), + i));
		}
	}
}

void SimpleMenu::event(MenuItem::menuEvent e) {
	if(items.size() <= 0) return;

	switch (items[position]->event(e))
	{
	case MenuItem::unhandled:
		if(e == MenuItem::up) position++;
		else if(e == MenuItem::down) position--;

		if(position < 0) position = items.size() - 1;
		if(position >= (int) items.size()) position = 0;

		items[position]->event(MenuItem::show);
		break;
	default:
		break;
	}
}

//MenuItem SimpleMenu::getIntValue(){
//
//	return 1;
//
//
//}

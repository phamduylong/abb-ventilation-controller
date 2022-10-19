/*
 * MenuItem.cpp
 *
 *  Created on: 1.2.2016
 *      Author: krl
 */

#include "MenuItem.h"


MenuItem::MenuItem(PropertyEdit *property): pe(property) {

}

MenuItem::~MenuItem() {

}
const char* MenuItem::getName() {

return pe->getTitle();

}
MenuItem::menuItemEvent MenuItem::event(menuEvent e) {
	MenuItem::menuItemEvent res = handled;
	switch(e) {
	case ok:
		if(pe->getFocus() && pe->getStatus()) {
			pe->accept();
			pe->setFocus(false);
			res = got_modified;
		}
		else if(!pe->getStatus()){
			pe->setFocus(false);
		}
		else if(!pe->getFocus()){
			pe->setFocus(true);
		}
		break;
	case back:
		if(pe->getFocus()) {
			pe->cancel();
			pe->setFocus(false);
		}
		else {
			res = unhandled;
		}
		break;
	case show:
		break;
	case up:
		if(pe->getFocus()) pe->increment();
		else res = unhandled;
		break;
	case down:
		if(pe->getFocus()) pe->decrement();
		else res = unhandled;
		break;
	}
	if(res) pe->display();

	return res;
}



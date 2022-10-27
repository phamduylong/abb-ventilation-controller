#ifndef MENUITEM_H_
#define MENUITEM_H_

#include "PropertyEdit.h"
#include "LpcUart.h"


class MenuItem {
public:
	enum menuEvent {
		up,
		down,
		ok,
		back,
		show
	};
	enum menuItemEvent {
		unhandled,
		handled,
		got_modified
	};

	MenuItem(PropertyEdit *property);
	virtual ~MenuItem();
	virtual MenuItem::menuItemEvent event(menuEvent e);
	const char* getName();
private:
	PropertyEdit *pe;

};
#endif /* MENUITEM_H_ */

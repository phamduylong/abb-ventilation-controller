#ifndef SIMPLEMENU_H_
#define SIMPLEMENU_H_

#include <algorithm>
#include <vector>


#include "MenuItem.h"
#include "IntegerEdit.h"
#include "LpcUart.h"

class SimpleMenu {
public:
	SimpleMenu();
	virtual ~SimpleMenu();
	void addItem(MenuItem *item);
	bool event(MenuItem::menuEvent e);
	void deleteItem(MenuItem *item);
//	MenuItem getIntValue();
private:
	std::vector<MenuItem *> items;
	int position;
	int counter;

};

#endif /* SIMPLEMENU_H_ */

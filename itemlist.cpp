#include "itemlist.h"
#include <vector>
using namespace std;

vector<Item> listOfItems() {
    return {
        Item("Health Potion", 0, 0, 50, 80, false),
        Item("Mega Potion", 0, 0, 150, 200, false),
        Item("Revive Stone", 0, 0, 0, 400, false)
    };
}
// default, start from 0, not owned, fixed items
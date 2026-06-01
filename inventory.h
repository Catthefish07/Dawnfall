#ifndef INVENTORY_H
#define INVENTORY_H

#include "item.h"
#include <string>
#include <vector>
using namespace std;

static const int maxQuantity = 100; // sets max total of items at 100

class Inventory
{
public:
    Inventory();
    void addItems(Item);
    void removeItems(string, int);
    void useItem(string, Character*);
    int getQuantity(string); // tracks quantity of items in shop
    void displayInventory();
    vector<Item> getItems();

private:
    vector<Item> items;
};

#endif // INVENTORY_H

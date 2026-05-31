#ifndef INVENTORY_H
#define INVENTORY_H

#include "item.h"
#include "character.h"
#include <string>
#include <vector>
using namespace std;

static const int maxQuantity = 100;

class Inventory
{
public:
    Inventory();
    void addItems(Item);
    void removeItems(string, int);
    void useItem(string, Character*);
    int getQuantity(string);
    void displayInventory();
    vector<Item> getItems();

private:
    vector<Item> items;
};

#endif // INVENTORY_H

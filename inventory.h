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
    void useItem(string, class Character*);
    void setQuantity(string itemName, int qty);
    int getQuantity(string); // tracks quantity of items in shop
    void displayInventory();
    vector<Item> getItems();

    // Added function to save/use the coins achieved after a battle
    void addCoins(int amount);
    bool spendCoins(int amount);
    int  getCoins() const;
    void setCoins(int amount);

private:
    vector<Item> items;
    int coins = 0;
};

#endif // INVENTORY_H

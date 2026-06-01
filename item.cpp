#include "item.h"
#include <iostream>
using namespace std;


void printItem(const Item& item) {
    cout << item.name << endl;
    cout << "Quantity : " << item.quantity << endl;

    if(item.name == "Health Potion")
        cout << "A consumable item to restore 50 health points during combat." << endl;
    else if(item.name == "Mega Potion")
        cout << "A consumable item to restore 150 health points during combat." << endl;
    else if(item.name == "Revive Stone")
        cout << "A consumable item to revive a character and restores 50% of their max health points during combat." << endl;

    cout << "Price : " << item.price << "coins" << endl;
    if(item.isOwned == true)
        cout << "Owned : " << item.quantity << "x" << endl;
    else if(item.isOwned == false)
        cout << "Item not yet own." << endl;
}


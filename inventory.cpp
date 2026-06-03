#include "inventory.h"
#include "itemlist.h"
#include "character.h"
#include <iostream>
#include <vector>
using namespace std;

Inventory::Inventory(){
    items = listOfItems();
}

void Inventory::addItems(Item item){
    for(Item& i : items) {
        if(i.name == item.name) { // check item name
            if(i.quantity + item.quantity > maxQuantity) { // if exist, check quantity if exceeds or no
                i.quantity = maxQuantity; // any type stored in inventory totally max at 100
                cout << "Inventory Full!" << endl;
            } else {
                i.quantity += item.quantity; // adds new item to inventory
                i.isOwned = true;
            }
            return;
        }
    }
    cout << item.name << " is not a valid item." << endl; // if item name entered not valid
}

void Inventory::removeItems(string itemName, int quantity){
    for(Item& i : items) {
        if(i.name == itemName) { // checks item name first
            i.quantity -= quantity; // removes item if name match
            if(i.quantity <= 0) {
                i.quantity = 0;
                i.isOwned = false; // no longer owns anything
            }
            return;
        }
    }
    cout << itemName << " not found!" << endl; // if item name does not matches anything
}

void Inventory::useItem(string itemName, Character* c){
    for(Item& i : items) {
        if(i.name == itemName && i.quantity > 0 && i.isOwned == true){ // checks first
            if(i.name == "Revive Stone") {
                c->heal(c->getMaxHP() * 0.5); // revive stone formula
                removeItems(itemName, 1); // deletes the item bcs have been used
                cout << "You revived! Restore 50% health points!" << endl;
            } else if(i.name == "Health Potion" || i.name == "Mega Potion") {
                c->heal(i.effect);
                removeItems(itemName, 1);
                cout << itemName << " used, restores " << i.effect << " health points!" << endl;
            }
            return;
        }
    }
    cout << itemName << " not owned!" << endl; // if not valid
}

void Inventory::setQuantity(string itemName, int qty){
    for(Item& i : items){
        if(i.name == itemName){
            i.quantity = qty;
            i.isOwned = (qty > 0); // to mark that is owned is true
        }
    }
}

int Inventory::getQuantity(string itemName){
    for(Item& i : items){
        if(i.name == itemName)
            return i.quantity;
    }
    cout << itemName << " not in inventory!" << endl;
    return 0;
}

vector<Item> Inventory::getItems() {
    return items;
}

void Inventory::displayInventory() { // if already loop through getItems() to show everything, no need this if UI handles it
    if(items.empty()) {             // might only for testing
        cout << "Inventory is empty!" << endl;
        return;
    }
    cout << "=== INVENTORY ===" << endl;
    for(Item& i : items) {
        if(i.isOwned && i.quantity > 0) {
            cout << i.name << " x" << i.quantity << endl;
        }
    }
}

// Added function to save/use gold from battle into inventory
// This is seperate from the usable items
void Inventory::addCoins(int amount) {
    coins += amount;
}

int  Inventory::getCoins() const {
    return coins;
}

bool Inventory::spendCoins(int amount) {
    if (amount > coins) {
        return false;
    }
    coins -= amount;
    return true;
}

void Inventory::setCoins(int amount) {
    coins = amount;
}

// printItem — one item detail
//printItem(healthPotion);
// output:
// Health Potion
// Quantity : 3
// A consumable item to restore 50 health points during combat.
// Price : 80 coins
// Owned : 3x

// displayInventory — whole list
//inventory.displayInventory();
// output:
// Health Potion x3
// Mega Potion x1
// Revive Stone x0

//idk man ui handles it later, just giving options. :D

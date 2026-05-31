#ifndef ITEM_H
#define ITEM_H

#include <string>
using namespace std;

enum itemTarget{Single = 0, Party = 1};

struct Item // list of the required information for each items
{
    string name;
    int quantity;
    int target;// 0 for single, 1 for party
    int effect; // the effect formula whenever the player receive heal
    int price; // price of each item if sold in shop
    bool isOwned; // true if player owns and default is false if player doesn't have the item

    Item(string n, int q, int tr, int e, int p, bool iO) : name(n), quantity(q),target(tr), effect(e), price(p), isOwned(iO) {}

public:
    Item(){
        name = "";
        quantity = 0;
        target = 0;
        effect = 0;
        price = 0;
        isOwned = false;
    } // default constructor, sets to this if theres no certain item in the inventory
    // we also declare the default in the vector<Item>, but leaving this is fine(?)
};

#endif // ITEM_H

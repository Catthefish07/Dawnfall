#ifndef SHOP_H
#define SHOP_H

#include "inventory.h"
#include "itemlist.h"
#include <vector>
#include <string>
using namespace std;

struct CharacterShop
{
    string name;
    int price;
    int worldRequired;
    bool isFree;
    bool isOwned;
};

class Shop
{
public:
    Shop();
    bool buyItem(string itemName, int, Inventory&, int&);
    vector<Item> itemStock = listOfItems(); // from itemlist.h
    bool buyCharacter(string, int&, int);
    bool isCharacterUnlocked(string) const; // to show can buy or already owned
    bool isCharacterAvailable(string, int) const; // to show wether character is locked or can buy
    void unlockCharacter(string); // called by savemanager after loadgame to restore bought characters
    vector<CharacterShop> getCharacterStock() const;
    void setCharacterUnlocked(int, bool); // so that every time player loads the game, it doesnt reset to not owned, linked to playerrecord later
    bool getCharacterUnlocked(int) const; // before saving to write current unlock state, needs to read to save it later
    static const int maxPerBuy = 10;

private:
    vector<CharacterShop> characterStock;
    void loadCharacterStock();

};

#endif // SHOP_H

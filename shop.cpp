#include "shop.h"
#include <iostream>
using namespace std;

Shop::Shop() {
    loadCharacterStock();
}

void Shop::loadCharacterStock() {
    characterStock = {
        {"Joy", 0, 1, true, true},
        {"Ethan", 0, 1, true, true},
        {"Hubert", 0, 1, true, true},
        {"Lynn", 500, 2, false, false},
        {"Ben", 500, 2, false, false},
        {"Cedric", 500, 2, false, false},
        {"Kae", 500, 3, false, false},
        {"Zey", 500, 3, false, false},
        {"Anak Agung", 2000, 4, false, false},
        };
}

bool Shop::buyItem(string itemName, int qty, Inventory& iv, int& playerCoins){
    if(qty < 1){
        cout << "You can't buy 0 items :(" << endl;
        return false;
    } else if(qty > maxPerBuy){
        cout << "You only can buy 10 items at max!" << endl;
        return false;
    }

    vector<Item> itemStock = listOfItems();
for(Item& i : itemStock){
    if(i.name == itemName){
        int totalCost = i.price * qty;

        if(playerCoins < totalCost){
            cout << "Not enough coins!" << endl;
            return false;
        }

        int totalOwned = iv.getQuantity("Health Potion") + iv.getQuantity("Mega Potion") + iv.getQuantity("Revive Stone");

        if(totalOwned + qty > maxQuantity){
            totalOwned = totalOwned;
            cout << "Inventory already full!" << endl;
            return false;
        }

        playerCoins -= totalCost; // minus total cost from total coins player had

        Item add = i; // copy of i from the vector, same properties
        add.quantity = 1; // set quantity to 1
        for(int j = 0; j < qty; j++){
            iv.addItems(add); // add 1 each loop
        }
        cout << itemName << " " << qty << "x purchased!" << endl;
        return true;
    }
}

cout << itemName << " not valid!" << endl;
return false;
}

bool Shop::buyCharacter(string characterName, int& playerCoins, int currentWorld){
    for(CharacterShop& c : characterStock){ // loops through each character in shop
        if(c.name == characterName){
            if(c.isOwned == true){
                cout << "Character owned!" << endl;
                return false; // cant buy bcs already own
            }
            if(currentWorld < c.worldRequired){ // needs currentWorld to be higher than world required
                cout << c.name << " is locked. Need to clear world " << c.worldRequired << " first!" << endl;
                return false;
            }
            if(playerCoins < c.price){
                cout << "Not enough coins!" << endl;
                return false;
            }
            playerCoins -= c.price; // playerCoins automatically minus the character price if can buy
            c.isOwned = true;
            cout << c.name << " unlocked!" << endl;
            return true;
        }
    }
    cout << characterName << " not found!" << endl; // not valid characterName
    return false;
}


bool Shop::isCharacterUnlocked(string characterName) const{
    for (const CharacterShop& c: characterStock){
        if(c.name == characterName)
            return c.isOwned; // if isOwned true then already unclocked
    }
    return false;
}

bool Shop::isCharacterAvailable(string characterName, int currentWorld) const {
    for(const CharacterShop& c : characterStock){
        if(c.name == characterName)
            return currentWorld >= c.worldRequired; // if currentworld is bigger than world required then that character is available
    }
    return false;
}

void Shop::unlockCharacter(string characterName) {
    for(CharacterShop& c : characterStock){
        if(c.name == characterName){
            c.isOwned = true;
            return;
        }
    }
}

vector<CharacterShop> Shop::getCharacterStock() const{
    return characterStock;
}

void Shop::setCharacterUnlocked(int index, bool unlocked){
    if(index < 0 || index >= (int)characterStock.size())
        characterStock[index].isOwned = unlocked; // index based on the list declared before in characterStock
}

bool Shop::getCharacterUnlocked(int index) const{
    if(index < 0 || index >= (int)characterStock.size())
        return false; // if index doesnt match any character
    return characterStock[index].isOwned;
}


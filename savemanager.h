#ifndef SAVEMANAGER_H
#define SAVEMANAGER_H

#include "inventory.h"
#include "shop.h"
#include "partymanager.h"
#include <string>
#include <vector>

using namespace std;

struct PlayerRecord
{
    char playerUsername[25];
    int avatarIndex;
    int coins;
    int currentChapter; // last saved chapter
    int characterLevels[9]; // each character levels
    bool characterUnlocked[9]; //  true if unlocked, false if locked
    bool occupied; // slot has save or not
    int partyIndexes[4]; // character currently in party
};

class SaveManager
{
public:
    void saveGame(int slot, PlayerRecord player, Inventory& iv, Shop& shop, PartyManager& pm); // saves data of the whole game
    PlayerRecord loadGame(int slot, Inventory& iv, Shop& shop, PartyManager& pm); // restores back data when player loads game

    bool isSlotEmpty(int slot);
    void deleteSlot(int slot); // erase slot

    void saveInventory(Inventory& iv); // save inventory quantities to items.dat
    void loadInventory(Inventory& iv); // restores

    void addBattleLog(string text); // adds a line like "Snake defeated. Coins +..."
    vector<string> readBattleLog();

    void addStoryLog(string text); // adds a line of text, when character joined the party, chapter 1 accomplished.."
    vector<string> readStoryLog();

private:
    string saveFile = "players.dat"; // saves player's account data/progress data
    string itemFile = "items.dat"; // for inventory quantities
    string battleFile = "battlelog.txt"; // text file for battle history
    string storyFile = "storylog.txt"; // text file for story progress history

    int totalSlots = 3; // totalslots to save progress on story mode
};

#endif // SAVEMANAGER_H

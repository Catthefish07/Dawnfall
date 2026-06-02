#include "savemanager.h"
#include "inventory.h"
#include "shop.h"

#include <fstream>
#include <iostream>
using namespace std;

void SaveManager::saveGame(int slot, PlayerRecord player, Inventory& iv, Shop& shop){
    if(slot < 0 || slot >= totalSlots){ // checks valid slot first (3 slots)
        cout << "Invalid slot." << endl;
        return;
    }

    for(int i = 0; i < 9; i++){ // loop all 9 characters
        player.characterUnlocked[i] = shop.getCharacterUnlocked(i); // shop -> playerrecord needs update too
    }

    saveInventory(iv); // saves items amount

    fstream file(saveFile, ios::in | ios::out | ios::binary); // savefile

    if(!file){
        ofstream newFile(saveFile, ios::binary); // if file cant be opened, make new file

        PlayerRecord empty = {}; // empty it first
        empty.occupied = false; // empty slot

        for(int i = 0; i < totalSlots; i++){
            newFile.write((char*)&empty, sizeof(PlayerRecord)); // writes the empty playerrecord
        }
        newFile.close();

        file.open(saveFile, ios::in | ios::out | ios::binary); // reopen file, now works
    }

    file.seekp(slot * sizeof(PlayerRecord)); // move to correct slot for write using seekp
    file.write((char*)&player, sizeof(PlayerRecord)); // writes it after
    file.close();
    cout << "Game Saved!" << endl;
}

PlayerRecord SaveManager::loadGame(int slot, Inventory& iv, Shop& shop){
    PlayerRecord player = {};
    player.occupied = false; // default empty slot

    if(slot < 0 || slot >= totalSlots){
        return player; // invalid slot
    }

    ifstream file(saveFile, ios::binary); // open savefile

    if(!file){ // checks file first
        return player;
    }

    file.seekg(slot * sizeof(PlayerRecord)); // get slot for read
    file.read((char*)&player, sizeof(PlayerRecord)); // load by reading last saved slot

    file.close();

    loadInventory(iv); // read inventory from items.dat then put back to inventory or like to restore

    for(int i = 0; i < 9; i++) {
        shop.setCharacterUnlocked(i, player.characterUnlocked[i]); // restore character unlocked
    }

    return player;
}

bool SaveManager::isSlotEmpty(int slot){
    if(slot < 0 || slot >= totalSlots){
        return true; // it is empty, bcs invalid slot
    }

    ifstream file(saveFile, ios::binary);

    if(!file){
        return true; // cant open file
    }

    PlayerRecord player = {}; // temporary

    file.seekg(slot * sizeof(PlayerRecord));
    file.read((char*)&player, sizeof(PlayerRecord)); // contains slot data
    file.close();

    return !player.occupied; // reverse of occupied, if occupied = true then isslotempty is false, which means not empty
}

void SaveManager::deleteSlot(int slot){
    PlayerRecord empty = {}; // make empty player first
    empty.occupied = false;

    fstream file(saveFile, ios::in | ios::out | ios::binary);

    if(!file){
        return;
    }

    file.seekp(slot * sizeof(PlayerRecord));
    file.write((char*)&empty, sizeof(PlayerRecord)); // write it to empty
    file.close();
    cout << "Slot deleted!" << endl;
}

void SaveManager::saveInventory(Inventory& iv){
    ofstream file(itemFile, ios::binary); // make itemfile to write(items.dat)

    int h = iv.getQuantity("Health Potion"); // get amounts of all items
    int m = iv.getQuantity("Mega Potion");
    int r = iv.getQuantity("Revive Stone");

    file.write((char*)&h, sizeof(int)); // write and save amount to itemfile
    file.write((char*)&m, sizeof(int));
    file.write((char*)&r, sizeof(int));

    file.close();
}

void SaveManager::loadInventory(Inventory& iv){
    ifstream file(itemFile, ios::binary); // make ifstream itemfile to read/load

    if(!file){
        return;
    }

    int h = 0; // variables to hold amounts
    int m = 0;
    int r = 0;

    file.read((char*)&h, sizeof(int)); // after read, amount changes, not 0 anymore
    file.read((char*)&m, sizeof(int));
    file.read((char*)&r, sizeof(int));

    file.close();

    iv.setQuantity("Health Potion", h); // copy to inventory
    iv.setQuantity("Mega Potion", m);
    iv.setQuantity("Revive Stone", r);
}

void SaveManager::addBattleLog(string text){
    ofstream file(battleFile, ios::app); // create battlelog, writes at end of file, keep most updated

    file << text << endl; // add the text

    file.close();
}

vector<string> SaveManager::readBattleLog(){
    vector<string> logs;

    ifstream file(battleFile); // reads, open the file

    string line;

    while(getline(file, line)){ // loop until end of file
        logs.push_back(line); // add line to end of vector
    }

    file.close();
    return logs; // returns all logs
}

void SaveManager::addStoryLog(string text){
    ofstream file(storyFile, ios::app); // create storylog, writes at the end of file too

    file << text << endl;

    file.close();
}

vector<string> SaveManager::readStoryLog(){
    vector<string> logs;

    ifstream file(storyFile);

    string line;

    while(getline(file, line)){
        logs.push_back(line);
    }

    file.close();
    return logs; // same mechanism as readbattlelog
}
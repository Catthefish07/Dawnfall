#include "partymanager.h"

#include <iostream>
using namespace std;

bool PartyManager::addCharacter(Character* c){
    if(isFull()){
        cout << "Party is full!" << endl;
        return false;
    }

    if(isInParty(c)){
        cout << c -> getName() << " already in party!" << endl;
        return false;
    }

    if(!isUnlocked(c)){
        cout << c -> getName() << " is locked!" << endl;
        return false;
    }

    party.push_back(c); // adds character to vector(party)
    cout << c -> getName() << " joined the party!" << endl;
    return true;
}

void PartyManager::removeCharacter(int slot){

    if(slot < 0 || slot >= party.size()){
        return;
    }

    cout << party[slot] -> getName() << " removed from party." << endl;

    party.erase(party.begin() + slot); // remove character from party based on slot, counts from beginning
}

void PartyManager::swapCharacter(int slot, Character* newChar){
    if(slot < 0 || slot >= party.size()){
        return;
    }

    if(isInParty(newChar)){
        cout << "Character already in party!" << endl;
        return;
    }

    if(!isUnlocked(newChar)){
        cout << "Character is locked!" << endl;
        return;
    }

    cout << party[slot] -> getName() << " swapped with " << newChar -> getName() << endl;

    party[slot] = newChar; // vector slot replaced
}

vector<Character*> PartyManager::getParty(){
    return party; // return full party
}

bool PartyManager::isFull(){
    return party.size() >= maxPartySize; // if more than 4 then full
}

bool PartyManager::isEmpty(){
    return party.empty(); // true if empty
}

void PartyManager::unlockCharacter(Character* c){
    if(isUnlocked(c)){
        return;
    }

    unlockedCharacters.push_back(c); // add into uncloked characters if players buy

    cout << c->getName()  << " unlocked!" << endl;
}

bool PartyManager::isInParty(Character* c){
    for(Character* member : party){ // loop through all party members
        if(member == c){ // if the same address then same character
            return true;
        }
    }
    return false;
}

bool PartyManager::isUnlocked(Character* c){
    for(Character* member : unlockedCharacters){
        if(member == c){
            return true;
        }
    }
    return false;
}

vector<Character*> PartyManager::getUnlockedCharacters(){
    return unlockedCharacters; // get all unlocked characters player had (the list)
}

void PartyManager::clearParty(){
    party.clear(); // so that when loads, the memory clears first, so that it doesnt doubles
}
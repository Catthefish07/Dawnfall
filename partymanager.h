#ifndef PARTYMANAGER_H
#define PARTYMANAGER_H

#include <vector>
#include "character.h"
using namespace std;

class PartyManager
{
public:
    bool addCharacter(Character* c);
    void removeCharacter(int slot);
    void swapCharacter(int slot, Character* newChar);
    bool isFull();
    bool isEmpty();
    void unlockCharacter(Character* c);
    bool isInParty(Character* c);
    bool isUnlocked(Character* c);
    void clearParty();
    vector<Character*> getParty();
    vector<Character*> getUnlockedCharacters();

private:
    vector<Character*> party;
    vector<Character*> unlockedCharacters;
    int maxPartySize = 4;
};

#endif // PARTYMANAGER_H

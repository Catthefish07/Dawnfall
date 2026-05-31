#ifndef WARRIOR_H
#define WARRIOR_H

#include "character.h"
#include <string>

class Warrior : public Character
{

public:
    Warrior(std::string name, int maxHP, int attack, int defense, int speed);\

    int dealDamage() const override;
    void takeDamage(int amount) override;
};

#endif // WARRIOR_H

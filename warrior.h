#ifndef WARRIOR_H
#define WARRIOR_H

#include "meleeattack.h"
#include <string>

class Warrior : public MeleeAttack
{
public:
    Warrior(std::string name, int maxHP, int attack, int defense, int speed);\

    int dealDamage() const override;
};

#endif // WARRIOR_H

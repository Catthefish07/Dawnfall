#ifndef ASSASSIN_H
#define ASSASSIN_H

#include "meleeattack.h"

class Assassin : public MeleeAttack
{
public:
    Assassin(std::string name, int maxHP, int attack, int defense, int speed);

    int dealDamage() const override;
};

#endif // ASSASSIN_H

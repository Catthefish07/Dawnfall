#ifndef TANK_H
#define TANK_H

#include "meleeattack.h"

class Tank : public MeleeAttack
{
public:
    Tank(std::string name, int maxHP, int attack, int defense, int speed);

    int dealDamage() const override;
};

#endif

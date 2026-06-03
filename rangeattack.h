#ifndef RANGEATTACK_H
#define RANGEATTACK_H

#include "character.h"

class RangeAttack : public Character
{
public:
    RangeAttack(std::string name, int maxHP, int attack, int defense, int speed);
    AttackType getAttackType() const override;
};

#endif // RANGEATTACK_H

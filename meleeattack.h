#ifndef MELEEATTACK_H
#define MELEEATTACK_H

#include "character.h"

class MeleeAttack : public Character
{
public:
    MeleeAttack(std::string name, int maxHP, int attack, int defense, int speed);
    AttackType getAttackType() const override;
};

#endif // MELEEATTACK_H

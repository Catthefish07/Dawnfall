#ifndef MAGE_H
#define MAGE_H

#include "rangeattack.h"

class Mage : public RangeAttack
{
public:
    Mage(std::string name, int maxHP, int attack, int defense, int speed);

    int dealDamage() const override;
};

#endif // MAGE_H

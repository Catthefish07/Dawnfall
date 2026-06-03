#ifndef ARCHER_H
#define ARCHER_H

#include "rangeattack.h"

class Archer : public RangeAttack {
public:
    Archer(std::string name, int maxHP, int attack, int defense, int speed);

    int dealDamage() const override;
};

#endif // ARCHER_H

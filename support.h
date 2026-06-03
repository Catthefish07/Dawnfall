#ifndef SUPPORT_H
#define SUPPORT_H

#include "rangeattack.h"

class Support : public RangeAttack {
public:
    Support(std::string name, int maxHP, int attack, int defense, int speed);

    int dealDamage() const override;
};

#endif // SUPPORT_H

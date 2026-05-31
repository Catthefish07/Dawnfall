#ifndef TANK_H
#define TANK_H

#include "character.h"

class Tank : public Character {
public:
    Tank(std::string name, int maxHP, int attack, int defense, int speed);

    int dealDamage() const override;
};

#endif
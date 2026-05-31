#ifndef ARCHER_H
#define ARCHER_H

#include "character.h"

class Archer : public Character {
public:
    Archer(std::string name, int maxHP, int attack, int defense, int speed);

    int dealDamage() const override;
};

#endif // ARCHER_H
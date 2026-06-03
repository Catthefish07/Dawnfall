#include "rangeattack.h"

RangeAttack::RangeAttack(std::string name, int maxHP, int attack, int defense, int speed) : Character(name, maxHP, attack, defense, speed) {}

AttackType RangeAttack::getAttackType() const{
    return RANGE;
}
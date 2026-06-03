#include "meleeattack.h"

MeleeAttack::MeleeAttack(std::string name, int maxHP, int attack, int defense, int speed) : Character(name, maxHP, attack, defense, speed) {}

AttackType MeleeAttack::getAttackType() const{
    return MELEE;
}
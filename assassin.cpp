#include "assassin.h"

Assassin::Assassin(std::string name, int maxHP, int attack, int defense, int speed) : MeleeAttack(name, maxHP, attack, defense, speed) {
    skills.push_back(Skill("Silent Petal", DAMAGE,  30, 4));
}

int Assassin::dealDamage() const {
    return (int)(attack * 1.8);
}
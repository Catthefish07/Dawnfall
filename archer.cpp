#include "archer.h"

Archer::Archer(std::string name, int maxHP, int attack, int defense, int speed) : Character(name, maxHP, attack, defense, speed) {
    skills.push_back(Skill("Precise Shot", DAMAGE,  35, 2));
}

int Archer::dealDamage() const {
    return attack;
}
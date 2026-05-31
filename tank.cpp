#include "tank.h"

Tank::Tank(std::string name, int maxHP, int attack, int defense, int speed)
    : Character(name, maxHP, attack, defense, speed) {
    skills.push_back(Skill("Shield Bash", 20, 2));
    skills.push_back(Skill("Taunt", 0, 3));
    skills.push_back(Skill("Fortify", 0, 4));
}

int Tank::dealDamage() const {
    return attack;
}
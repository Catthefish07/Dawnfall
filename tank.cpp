#include "tank.h"

Tank::Tank(std::string name, int maxHP, int attack, int defense, int speed) : Character(name, maxHP, attack, defense, speed) {
    skills.push_back(Skill("Sun Guard", DEFEND, 20, 4));
}

int Tank::dealDamage() const {
    return attack;
}
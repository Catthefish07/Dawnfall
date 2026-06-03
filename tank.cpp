#include "tank.h"

Tank::Tank(std::string name, int maxHP, int attack, int defense, int speed) : MeleeAttack(name, maxHP, attack, defense, speed) {
    skills.push_back(Skill("Sun Guard", DEFEND, 12, 4));
}

int Tank::dealDamage() const {
    return (int)(attack * 0.7);
}

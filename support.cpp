#include "support.h"

Support::Support(std::string name, int maxHP, int attack, int defense, int speed) : RangeAttack(name, maxHP, attack, defense, speed) {
    skills.push_back(Skill("Sun Guard", HEAL, 45, 3));
}

int Support::dealDamage() const {
    return (int)(attack * 0.6);
}
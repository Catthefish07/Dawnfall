#include "archer.h"

Archer::Archer(std::string name, int maxHP, int attack, int defense, int speed) : RangeAttack(name, maxHP, attack, defense, speed) {
    skills.push_back(Skill("Precise Shot", DAMAGE,  20, 2));
}

int Archer::dealDamage() const {
    return (int)(attack * 1.3);
}

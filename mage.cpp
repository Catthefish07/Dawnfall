#include "mage.h"

Mage::Mage(std::string name, int maxHP, int attack, int defense, int speed) : RangeAttack(name, maxHP, attack, defense, speed){
    skills.push_back(Skill("Chaos Order", DAMAGE, 28, 4));
    skills.push_back(Skill("Moonlight", DAMAGE, 13, 2));
}

int Mage::dealDamage() const {
    return (int)(attack * 1.5);
}

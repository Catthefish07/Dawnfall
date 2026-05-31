#include "warrior.h"
#include "skill.h"
#include <string>
#include <cstdlib>

Warrior::Warrior(std::string name, int maxHP, int attack, int defense, int speed) : Character(name, maxHP, attack, defense, speed) {
    skills.push_back(Skill("Fury Strike", 30, 2));
    skills.push_back(Skill("Moon Blitz", 40, 3));
}

int Warrior::dealDamage() const {
    return attack;
}

void Warrior::takeDamage(int amount) {
    int realDamage = amount - defense;
    if (realDamage < 1) {
        realDamage = 1;
    }
    Character::takeDamage(realDamage);
}